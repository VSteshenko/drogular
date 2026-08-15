# Protecting Pages

## Problem

**Need to prevent guests or unauthorized users from reaching protected page data?**

Use `PageAuthSupport` at the beginning of the page lifecycle and stop page initialization immediately when a requirement fails.

The `auth_sample` example demonstrates two common cases:

- `/dashboard` requires a complete authenticated application identity;
- `/admin` requires authentication and then `role=admin`.

---

## Recommended Solution

Treat access checks as part of the page contract, not as presentation logic.

A protected page should follow this sequence:

```text
Page::onInit()
    │
    ▼
Require authentication
    │
    ├── denied ──► set loginRequired ──► return
    │
    ▼ allowed
Require authorization, if needed
    │
    ├── denied ──► set accessDenied ───► return
    │
    ▼ allowed
Load protected data
    │
    ▼
Render page
```

The important rule is that protected work happens only after every required check succeeds.

---

## Require Authentication at the Page Boundary

`PageAuthSupport::requireAuthentication()` checks the session associated with the current `RenderContext`.

```cpp
if (!drogular::PageAuthSupport::requireAuthentication(
        context
    )) {
    return;
}

// Safe to continue with work that only requires
// the framework authentication baseline.
```

When authentication fails, the helper sets:

```text
loginRequired = true
```

and returns `false`.

When authentication succeeds, it sets `loginRequired` to `false` and returns `true`.

The helper does **not** terminate `onInit()` automatically. The page must return explicitly.

This is the page contract:

```cpp
if (!requirement(context)) {
    return;
}

// Protected work begins here.
```

Do not perform database queries, service calls, or populate protected template values before this boundary.

---

## Framework Authentication Is a Baseline

Drogular's built-in authentication rule considers a resolved session authenticated when it contains `username`.

That is useful as a common framework baseline, but an application may require a stronger identity contract.

`auth_sample` requires both `username` and `role` before it considers its `AuthUser` complete.

```cpp
static std::optional<AuthUser> currentUser(
    drogular::RenderContext& context
) {
    const auto username =
        drogular::AuthSupport::sessionValue(
            context,
            "username"
        );

    const auto role =
        drogular::AuthSupport::sessionValue(
            context,
            "role"
        );

    if (!username.has_value() ||
        !role.has_value()) {
        return std::nullopt;
    }

    return AuthUser{
        .username = *username,
        .role = *role
    };
}
```

This distinction matters when a session is valid at the framework level but incomplete for the application.

---

## Wrap the Application Identity Contract

Rather than repeating identity reconstruction in every page, `auth_sample` defines `AuthSession::requireCurrentUser()`.

```cpp
static std::optional<AuthUser> requireCurrentUser(
    drogular::RenderContext& context
) {
    if (!drogular::PageAuthSupport::requireAuthentication(
            context
        )) {
        return std::nullopt;
    }

    const auto user = currentUser(context);

    if (!user.has_value()) {
        context.set("loginRequired", true);
    }

    return user;
}
```

This helper composes two requirements:

1. the request must satisfy Drogular's authentication baseline;
2. the session must contain the complete identity required by the application.

If either condition fails, the page treats the request as unauthenticated and stops.

This is preferable to sprinkling optional session lookups throughout page code.

---

## Protect a Dashboard Page

`DashboardPage` requires a complete current user before it writes any protected values into the render context.

```cpp
void onInit(
    drogular::RenderContext& context
) override {
    const auto currentUser =
        AuthSession::requireCurrentUser(context);

    if (!currentUser.has_value()) {
        return;
    }

    context.set("pageTitle", std::string("Dashboard"));
    context.set("username", currentUser->username);
    context.set(
        "isAdmin",
        currentUser->role == "admin"
    );
}
```

The important ordering is:

```text
requireCurrentUser()
        │
        ├── missing ──► return
        │
        ▼
use currentUser
```

Never reverse this order or dereference an optional user before the requirement succeeds.

---

## Add Role-Based Authorization

Authentication answers whether the page has a valid user. Authorization answers whether that user satisfies a specific access rule.

`AdminPage` first establishes the complete identity and only then checks the admin role:

```cpp
const auto currentUser =
    AuthSession::requireCurrentUser(context);

if (!currentUser.has_value()) {
    return;
}

if (!drogular::PageAuthSupport::requireSessionValue(
        context,
        "role",
        "admin"
    )) {
    return;
}

context.set("pageTitle", std::string("Admin"));
context.set("username", currentUser->username);
```

`requireSessionValue()` sets:

```text
accessDenied = true
```

when the value is missing or does not match the expected value, and returns `false`.

The sequence is deliberately:

```text
Authentication
      │
      ▼
Complete application identity
      │
      ▼
Authorization
      │
      ▼
Protected admin data
```

A guest is therefore an authentication failure, while an authenticated regular user is an authorization failure.

---

## Distinguish Login Required from Access Denied

The two render flags represent different outcomes:

| Condition | Render flag | Meaning |
|---|---|---|
| no authenticated identity | `loginRequired` | the user must authenticate |
| authenticated but missing required value | `accessDenied` | the user is known but not permitted |

The `auth_sample` admin template reflects this distinction:

```text
Guest
  └── Login required

Regular user
  └── Access Denied

Administrator
  └── Admin Panel
```

This separation is useful for both user experience and security reasoning.

Do not collapse authorization failure into “not logged in” when the application already knows who the user is.

---

## Keep Security Decisions Out of Templates

Templates may display the result of a page access decision:

```html
@if(loginRequired)
<h2>Login required</h2>
@else
<h2>Dashboard</h2>
@endif
```

and:

```html
@if(accessDenied)
<h2>Access Denied</h2>
@endif
```

But the template is not the security boundary.

By the time rendering starts, page code must already have stopped before protected values were loaded.

Incorrect pattern:

```cpp
context.set("secretData", loadSecretData());

drogular::PageAuthSupport::requireAuthentication(context);
```

Even if the template hides `secretData`, the protected work has already happened.

Correct pattern:

```cpp
if (!drogular::PageAuthSupport::requireAuthentication(
        context
    )) {
    return;
}

context.set("secretData", loadSecretData());
```

Security belongs in C++ control flow; templates only present the resulting state.

---

## Fail Closed on Incomplete Sessions

A server-side session can exist without containing every identity field the application expects.

For example:

```text
username = admin
role     = <missing>
```

Drogular's baseline authentication check sees `username`, but `auth_sample` cannot construct a complete `AuthUser`.

`requireCurrentUser()` therefore sets `loginRequired` and returns `std::nullopt`.

The page then returns before accessing protected state.

```text
Session exists
    │
    ▼
Framework baseline valid?
    │
    ├── no ──► login required
    │
    ▼ yes
Application identity complete?
    │
    ├── no ──► login required
    │
    ▼ yes
Continue
```

Treat missing or malformed identity state as denial, not as a partial success.

---

## Do Not Use `isAuthenticated()` as the Only Contract When You Need a User

A boolean check can answer whether some authentication condition holds, but it does not give the page a validated application principal.

Avoid patterns like:

```cpp
if (AuthSession::isAuthenticated(context)) {
    const auto user = AuthSession::currentUser(context);
    // Additional assumptions about user follow here.
}
```

When the page needs the user, prefer one operation that both validates and returns it:

```cpp
const auto currentUser =
    AuthSession::requireCurrentUser(context);

if (!currentUser.has_value()) {
    return;
}
```

This reduces the chance that the check and the data being consumed use different definitions of “authenticated”.

---

## Page Protection Is Not Route Middleware

`PageAuthSupport` protects page initialization through an explicit page-level contract.

It does not alter route registration:

```cpp
app.page<DashboardPage>("/dashboard");
app.page<AdminPage>("/admin");
```

and it does not automatically intercept a request before the page object runs.

The protection is expressed inside the page boundary:

```cpp
void onInit(
    drogular::RenderContext& context
) override {
    if (!/* required access condition */) {
        return;
    }

    // Protected page initialization.
}
```

This explicit contract keeps the access rule next to the page data it protects.

---

## Testing Protected Pages

Test the resulting security behavior, not only helper return values.

For a page that requires authentication, verify that a guest sees the denial state and does not see protected content:

```cpp
EXPECT_TRUE(
    drogular::test::contains(
        result.html,
        "Login required"
    )
);

EXPECT_FALSE(
    drogular::test::contains(
        result.html,
        "Welcome, admin."
    )
);
```

For an admin page, cover all three states:

```text
Guest          ──► Login required
Regular user   ──► Access Denied
Administrator  ──► Admin Panel
```

Also test incomplete identity state. `auth_sample` creates a session containing `username` but no `role` and verifies that the dashboard remains inaccessible.

This catches an important class of bugs where framework-level authentication succeeds but the application's principal is incomplete.

---

## Best Practices

- Put page access requirements at the beginning of `onInit()`.
- Return immediately when a requirement fails.
- Load protected data only after authentication and authorization succeed.
- Authenticate before checking roles or permissions.
- Define a complete application principal instead of repeatedly reading unrelated session keys.
- Use a helper such as `requireCurrentUser()` when pages need the validated user object.
- Treat missing or malformed identity state as denial.
- Keep authorization decisions in C++ page control flow, not in templates.
- Use `loginRequired` for authentication failure and `accessDenied` for authorization failure.
- Test guests, authorized users, unauthorized authenticated users, and incomplete sessions.

---

## Next Steps

Continue with **Protecting Actions** to apply the same boundary-first model to commands and POST handlers using `ActionAuthSupport`.

---

## See Also

### Example

- `examples/auth_sample/auth_session.hpp`
- `examples/auth_sample/dashboard_page.hpp`
- `examples/auth_sample/admin_page.hpp`
- `examples/auth_sample/templates/dashboard.html`
- `examples/auth_sample/templates/admin.html`

### Cookbook

- [Authentication Overview](overview.md)
- [Login & Logout](login-logout.md)

### API Reference

- [`AuthSupport`](../../reference/authentication-and-sessions/auth-support.md)
- [`PageAuthSupport`](../../reference/authentication-and-sessions/page-auth-support.md)
- [`Session`](../../reference/authentication-and-sessions/session.md)
- [`SessionStore`](../../reference/authentication-and-sessions/session-store.md)
