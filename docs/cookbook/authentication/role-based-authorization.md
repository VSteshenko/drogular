# Role-Based Authorization

## Problem

**Need to restrict pages or actions to users with a specific application role?**

Store the role as part of the authenticated session and apply the role requirement at the protected page or action boundary.

Drogular does not provide a built-in RBAC engine. It provides session primitives and authorization helpers that applications can compose into their own role model.

The `auth_sample` uses a simple rule:

```text
role = "admin"
```

An authenticated user with that session value may access the admin page. Other authenticated users are denied.

---

## Recommended Solution

Treat role checks as application policy layered on top of authentication.

The general sequence is:

```text
Request
   │
   ▼
Authenticate identity
   │
   ├── failed ──► login required
   │
   ▼
Evaluate application role policy
   │
   ├── failed ──► access denied
   │
   ▼
Execute protected work
```

Authentication must succeed before authorization is evaluated.

This keeps the two questions separate:

```text
Authentication
    └── Who is the current user?

Authorization
    └── Is this user allowed to perform this operation?
```

---

## Store the Role in the Authenticated Session

After successful credential validation, `auth_sample` stores the user's role together with the rest of the authenticated identity:

```cpp
session->set(
    "username",
    user->username
);

session->set(
    "role",
    user->role
);
```

The session therefore contains the authorization input used by later requests.

For the sample application, a complete identity contains both:

```text
username
role
```

The role is application data. Drogular does not define which role names exist or what they mean.

---

## Protect an Admin Page

For pages, use `PageAuthSupport` after establishing the application identity.

`auth_sample` protects `AdminPage` like this:

```cpp
void onInit(
    drogular::RenderContext& context
) override {
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
}
```

The ordering is deliberate:

```text
complete authenticated user
        ↓
role requirement
        ↓
protected admin data
```

A guest fails authentication and receives `loginRequired` state.

A regular authenticated user fails the role requirement and receives `accessDenied` state.

An administrator passes both requirements.

---

## Protect an Admin Action

For actions, use the equivalent `ActionAuthSupport` checks and return the denial result immediately.

```cpp
if (const auto denied =
        drogular::ActionAuthSupport::requireAuthentication(
            context
        )) {
    return *denied;
}

if (const auto denied =
        drogular::ActionAuthSupport::requireSessionValue(
            context,
            "role",
            "admin",
            "/dashboard?error=access_denied"
        )) {
    return *denied;
}

// Admin-only mutation starts here.
```

The role check belongs before validation, service lookup, or mutation.

This gives the action a clear authorization boundary:

```text
authentication
    ↓
authorization
    ↓
validation
    ↓
mutation
```

---

## `requireSessionValue()` Is a Primitive, Not an RBAC Engine

Both page and action helpers perform an exact session-value comparison.

Conceptually:

```text
session[key] == expectedValue
```

For example:

```cpp
requireSessionValue(
    context,
    "role",
    "admin"
);
```

This is intentionally small.

Drogular does **not** define:

- a `Role` framework type;
- role inheritance;
- permission graphs;
- role hierarchies;
- policy evaluation;
- database-backed role persistence;
- user-to-role assignment rules.

Those are application concerns.

The framework provides the request/session boundary on which those policies can be implemented.

---

## Keep Role Meaning in the Application

A role string should represent application policy, not framework behavior.

For example:

```text
admin
editor
viewer
```

are meaningful only if the application defines what each role may do.

Avoid spreading that meaning across unrelated pages and actions.

Instead, centralize repeated policy decisions in application-level helpers.

For example:

```cpp
class AuthorizationPolicy {
public:
    static bool requireAdmin(
        drogular::RenderContext& context
    ) {
        return drogular::PageAuthSupport::requireSessionValue(
            context,
            "role",
            "admin"
        );
    }
};
```

Then pages can express intent directly:

```cpp
if (!AuthorizationPolicy::requireAdmin(context)) {
    return;
}
```

For actions, use an equivalent helper returning `std::optional<ActionResult>`.

This keeps policy names application-specific while still using Drogular's authorization primitives underneath.

---

## Prefer Policy Helpers as Rules Become Richer

Direct `role=admin` checks are appropriate for small applications and examples.

As authorization becomes richer, avoid encoding complex policy directly into every handler.

Instead of repeating logic such as:

```cpp
if (role == "admin" ||
    (role == "editor" && ownsResource)) {
    // ...
}
```

move the decision into an application authorization service or policy object:

```cpp
if (!authorization.canEditProject(
        currentUser,
        project
    )) {
    return denyAccess(context);
}
```

The page or action still owns the boundary, but the application policy owns the decision.

This separation scales better than adding more session-value checks everywhere.

---

## Role Checks and Permissions Are Different Models

A role-based model asks:

```text
Does this user have role X?
```

A permission-based model asks:

```text
May this user perform operation Y?
```

For a small application, a role may map directly to permissions:

```text
admin
  ├── manage users
  ├── manage roles
  └── manage settings
```

For larger applications, authorization logic often becomes easier to maintain when handlers ask for capabilities rather than raw role names.

For example:

```cpp
authorization.canManageRoles(user)
```

instead of:

```cpp
user.role == "admin"
```

Drogular does not force either model. Both can be built on top of the authenticated session and application services.

---

## Do Not Trust Client-Supplied Roles

Authorization values must come from trusted server-side identity state.

Do not authorize requests using values from:

- form fields;
- query parameters;
- hidden inputs;
- JavaScript state;
- client-controlled headers.

For example, this is not authorization:

```cpp
const auto role = context.form<std::string>("role");

if (role == "admin") {
    // Unsafe: the client supplied the value.
}
```

Use the role stored in the authenticated server-side session or resolve authorization from a trusted application service.

---

## Fail Closed When the Role Is Missing

A missing role is not equivalent to a regular user unless the application explicitly defines that policy.

For a protected admin boundary:

```text
role = "admin"    → allow
role = "user"     → deny
role = <missing>   → deny
session missing    → deny
```

`PageAuthSupport::requireSessionValue()` and `ActionAuthSupport::requireSessionValue()` already follow this fail-closed behavior.

This matters when sessions are incomplete, stale, or malformed.

Do not write authorization code that treats missing security state as permission to continue.

---

## Avoid Checking Roles Only in the UI

Hiding an admin link is useful presentation behavior, but it is not authorization.

This is fine for navigation:

```text
@if(isAdmin)
    Show Admin link
@endif
```

but the target page and every privileged action must still enforce the role requirement themselves.

A user can call a URL directly without using the application's navigation.

The real security boundary is always the protected page or action.

---

## Keep Page and Action Policy Consistent

If an admin page displays a privileged operation, protect both sides:

```text
GET /admin
    └── require admin

POST /admin/update
    └── require admin
```

Protecting only the page leaves the mutation endpoint exposed.

Protecting only the action prevents unauthorized mutation, but may still expose protected page data.

A consistent authorization model applies the same application policy at every relevant boundary.

---

## Decide What Happens When Roles Change

When role values are copied into sessions, changing a user's role in persistent storage does not automatically rewrite already-created sessions.

Applications should decide their consistency policy explicitly.

Common approaches include:

- accept the session role until the session expires;
- revoke the user's active sessions when their role changes;
- store only a stable user identifier in the session and resolve current authorization from application storage on each request;
- use an authorization/version value to invalidate stale identity state.

Drogular does not choose this policy for the application.

For small examples such as `auth_sample`, storing the role directly in the session keeps the flow easy to understand. Production applications should choose a strategy appropriate to how quickly authorization changes must take effect.

---

## Test the Authorization Matrix

Authorization tests should cover the complete matrix, not only the success path.

For an admin-only page:

| Identity | Expected result |
|---|---|
| guest | login required |
| authenticated user | access denied |
| authenticated admin | page allowed |
| session missing `role` | denied |
| session with unexpected role | denied |

For an admin-only action:

| Identity | Expected result |
|---|---|
| guest | login redirect |
| authenticated user | access-denied redirect |
| authenticated admin | mutation allowed |
| session missing `role` | mutation not executed |

The most important action assertion is not merely the redirect.

Also verify that denied requests did not change application state.

---

## Common Mistakes

### Treating authentication as authorization

This is insufficient for an admin operation:

```cpp
if (!drogular::AuthSupport::isAuthenticated(context)) {
    return;
}

// Admin-only work.
```

Authentication establishes identity. It does not grant administrator privileges.

---

### Checking the role before establishing identity

Prefer:

```text
authenticate
    ↓
authorize
```

rather than treating a role value by itself as proof of a complete authenticated identity.

---

### Duplicating raw role strings everywhere

Repeated literals such as:

```cpp
"admin"
```

across many handlers eventually become application policy scattered throughout the codebase.

Centralize repeated rules when the application grows.

---

### Trusting presentation state

`isAdmin` in a template is for rendering decisions, not security enforcement.

Always repeat the authorization requirement at the page/action boundary that protects the resource or mutation.

---

### Treating missing authorization state as a default role

Security-sensitive state should fail closed.

Do not silently upgrade an incomplete session into an authorized identity.

---

## Choosing the Right Level of Abstraction

Use direct session-value checks when:

- the application has a very small role model;
- the rule is simple and stable;
- the code is an example or prototype.

Introduce application policy helpers when:

- the same role checks appear in many handlers;
- several roles may satisfy one operation;
- ownership or resource state affects access;
- role names should not leak throughout page/action code.

Introduce a dedicated authorization service when:

- permissions are persisted separately;
- authorization must be refreshed dynamically;
- policies depend on users, resources, organizations, or tenant boundaries;
- access decisions require more than session equality checks.

Drogular's helpers remain useful at each level because they provide the request/session integration, while the application remains free to choose its policy model.

---

## Summary

For role-based authorization in Drogular:

1. authenticate the user first;
2. store or resolve trusted authorization state on the server;
3. enforce the role requirement at the protected page or action boundary;
4. return immediately when authorization fails;
5. fail closed when role data is missing or unexpected;
6. protect privileged pages and their actions independently;
7. move repeated or complex rules into application-level policy helpers or services;
8. test both denial behavior and absence of unauthorized mutations.

`requireSessionValue()` is intentionally a small primitive. The application defines the actual RBAC model.

---

## See Also

- [Authentication Overview](overview.md)
- [Login & Logout](login-logout.md)
- [Protecting Pages](protecting-pages.md)
- [Protecting Actions](protecting-actions.md)
- [API Reference — Authentication & Sessions](../../reference/authentication-and-sessions/README.md)
- [API Reference — `PageAuthSupport`](../../reference/authentication-and-sessions/page-auth-support.md)
- [API Reference — `ActionAuthSupport`](../../reference/authentication-and-sessions/action-auth-support.md)
