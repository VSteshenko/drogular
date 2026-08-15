# Authentication Overview

## Problem

**Need to authenticate users and protect pages or actions without coupling application identity to the framework?**

This guide explains the authentication model provided by Drogular and the responsibilities that remain in application code.

Drogular provides session storage and reusable authentication/authorization helpers. The application defines users, validates credentials, decides which identity values belong in the session, and defines its own roles or permissions.

---

## Recommended Solution

Treat authentication as three separate responsibilities:

1. **authenticate credentials in application code;**
2. **store the established identity in a Drogular session;**
3. **enforce access requirements at the page or action boundary before protected work begins.**

The `auth_sample` example follows this model with an application-specific `AuthService`, `AuthSession`, and Drogular's session/authentication helpers.

```text
Credentials
    │
    ▼
Application AuthService
    │
    ▼
Authenticated User
    │
    ▼
SessionStore / Session
    │
    ├── PageAuthSupport
    │       └── Protected Page
    │
    └── ActionAuthSupport
            └── Protected Action
```

Drogular does not provide a user database or credential provider. This separation lets applications use a database, LDAP, OAuth/OpenID Connect integration, or another identity source without changing the framework authentication model.

---

## Authentication and Authorization

Authentication and authorization answer different questions.

**Authentication** establishes identity:

> Who is making this request?

**Authorization** checks whether that identity satisfies an access rule:

> Is this user allowed to access this resource or perform this operation?

In `auth_sample`, a successful login stores identity values such as `username` and `role` in the session. A protected page first requires an authenticated identity and then, when necessary, checks an authorization value such as `role=admin`.

```text
Request
   │
   ▼
Authenticated?
   │
   ├── no ──► deny / request login
   │
   ▼ yes
Authorized?
   │
   ├── no ──► deny access
   │
   ▼ yes
Protected work
```

Always establish authentication before evaluating authorization rules. This keeps missing, expired, or incomplete identity state from being mistaken for an authorization decision.

---

## Framework Responsibilities

Drogular provides the mechanics needed to associate requests with server-side session data and to enforce simple session-based requirements.

### `SessionStore` and `Session`

`SessionStore` creates, resolves, and removes in-memory sessions. Each `Session` stores string key/value data associated with one session identifier.

A request normally carries that identifier in the `session_id` cookie. Authentication helpers resolve the cookie through the registered `SessionStore`.

```text
HTTP Request
    │
    ▼
session_id cookie
    │
    ▼
SessionStore
    │
    ▼
Session
```

The built-in session store is in-memory. It does not provide persistence, expiration/TTL, or cross-process session sharing.

### `AuthSupport`

`AuthSupport` reads authentication-related values from the session associated with a render or action context.

For the built-in helpers, a request is considered authenticated when the resolved session contains a `username` value.

```cpp
if (drogular::AuthSupport::isAuthenticated(context)) {
    const auto role =
        drogular::AuthSupport::sessionValue(
            context,
            "role"
        );
}
```

Applications may build a stricter identity contract on top of these primitives. `auth_sample`, for example, requires both `username` and `role` before constructing its `AuthUser`.

### `PageAuthSupport`

`PageAuthSupport` adapts authentication checks to page rendering.

A protected page checks its requirements at the beginning of `onInit()` and returns immediately when a requirement fails.

```cpp
void onInit(
    drogular::RenderContext& context
) override {
    if (!drogular::PageAuthSupport::requireAuthentication(context)) {
        return;
    }

    // Populate protected page data only after authentication succeeds.
}
```

`requireAuthentication()` sets `loginRequired` in the render context when authentication fails. `requireSessionValue()` similarly sets `accessDenied` when a required session value does not match.

The helpers do not stop page execution automatically. The page owns the control-flow contract and must return before accessing protected data.

### `ActionAuthSupport`

`ActionAuthSupport` applies the same idea to actions. Instead of setting presentation flags, it returns an `ActionResult` redirect when a requirement fails.

```cpp
drogular::ActionResult handle(
    drogular::ActionContext& context
) override {
    if (const auto denied =
            drogular::ActionAuthSupport::requireAuthentication(context)) {
        return *denied;
    }

    // Perform the protected operation.
    return drogular::ActionResult::redirect("/dashboard");
}
```

A successful check returns `std::nullopt`, allowing the action to continue.

---

## Application Responsibilities

The application remains responsible for its identity and authorization model.

In `auth_sample`, `AuthService` validates credentials and returns an application-defined `AuthUser`:

```cpp
const auto user =
    authService->authenticate(
        context.requireForm<std::string>("username"),
        context.requireForm<std::string>("password")
    );

if (!user.has_value()) {
    return drogular::ActionResult::redirect("/login");
}
```

A real application may replace this service with database-backed authentication or an external identity provider.

The application also decides:

- which user fields are stored in the session;
- what constitutes a complete authenticated identity;
- which roles or permissions exist;
- how credentials are verified;
- how long sessions remain valid;
- whether sessions must be persisted or shared between processes;
- how CSRF and other application-level security controls are implemented.

Drogular intentionally does not define those policies.

---

## Establishing an Authenticated Session

After credentials are validated, create a fresh session and store only the identity values required by the application.

`auth_sample` removes any existing session before creating the authenticated one. Rotating the session identifier after login prevents an unauthenticated session identifier from being reused as the authenticated session.

```cpp
if (const auto existingSessionId =
        context.cookie("session_id")) {
    sessionStore->remove(*existingSessionId);
}

auto session = sessionStore->create();

session->set("user_id", std::to_string(user->id));
session->set("username", user->username);
session->set("role", user->role);
```

The response then sends the new session identifier as a cookie.

For the local HTTP example, `auth_sample` uses `HttpOnly` and `SameSite=Lax` while leaving `Secure` disabled. Production deployments served over HTTPS should enable the `Secure` attribute as part of their cookie policy.

---

## Requiring a Complete Application Identity

Framework authentication uses the presence of `username` as its shared baseline rule. Applications can require more.

`auth_sample` defines `AuthSession::currentUser()` to construct an `AuthUser` only when both `username` and `role` are present:

```cpp
if (!username.has_value() ||
    !role.has_value()) {
    return std::nullopt;
}

return AuthUser{
    .username = *username,
    .role = *role
};
```

Its `requireCurrentUser()` first applies the framework authentication requirement and then validates the complete application identity.

This is a useful boundary: framework helpers provide reusable session checks, while the application defines what a valid principal means for its own domain.

---

## Protecting a Page

`DashboardPage` establishes its authentication requirement before populating protected template data:

```cpp
const auto currentUser =
    AuthSession::requireCurrentUser(context);

if (!currentUser.has_value()) {
    return;
}

context.set("pageTitle", std::string("Dashboard"));
context.set("username", currentUser->username);
```

An admin-only page adds authorization after identity has been established:

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

// Populate admin-only data.
```

This ordering keeps protected work behind an explicit page boundary and distinguishes login failure from authorization failure.

---

## Fail Closed

Authentication code should stop protected work whenever the required identity state is missing or invalid.

`auth_sample` follows this rule for incomplete sessions. A session containing `username` but missing the application-required `role` does not produce a usable `AuthUser`; the page returns before reading protected data.

```text
Complete identity
      │
      ├── yes ──► continue
      │
      └── no  ──► stop protected work
```

Do not treat partial session state as implicitly trusted, and do not rely on templates to prevent access to data that the page has already loaded.

---

## Keep Security Decisions Out of Templates

Templates should render the result of an access decision, not make the decision themselves.

For pages, Drogular exposes values such as `loginRequired` and `accessDenied` through `RenderContext`. The page must still stop before it populates protected data.

This keeps the security boundary in C++ control flow:

```text
Page requirement
      │
      ├── denied ──► set render state ──► template displays denial
      │
      └── allowed ─► load protected data ─► template renders content
```

Changing a template therefore cannot accidentally bypass the page's access check.

---

## Security Model

The recommended Drogular authentication pattern follows these principles:

- **Authenticate before authorizing.** Establish a valid application identity before checking roles or permissions.
- **Fail closed.** Missing or incomplete identity state stops protected work.
- **Protect at the page/action boundary.** Check requirements before loading protected data or executing protected commands.
- **Keep identity application-defined.** Drogular provides session primitives; the application defines users and credentials.
- **Rotate the session after login.** Do not promote an existing unauthenticated session identifier into an authenticated session.
- **Destroy the server-side session on logout.** Also expire the client cookie.
- **Keep security decisions out of templates.** Templates present denial or content after C++ code has made the access decision.
- **Use production cookie settings in production.** In particular, enable `Secure` when the application is served over HTTPS.

These principles are demonstrated by `examples/auth_sample` and form the basis for the remaining Authentication Cookbook guides.

---

## Next Steps

Continue with:

1. **Login & Logout** — validate credentials, rotate sessions, issue cookies, and terminate sessions safely.
2. **Protecting Pages** — establish page-level authentication requirements with `PageAuthSupport` and application-specific identity helpers.
3. **Protecting Actions** — stop protected commands with `ActionAuthSupport`.
4. **Role-Based Authorization** — apply role requirements after authentication succeeds.
5. **Session Management** — understand session lifetime, storage, cookies, and deployment limitations.
6. **Custom Authentication Backend** — connect application-specific credential and identity infrastructure.
7. **Security Best Practices** — review production considerations beyond the basic sample.

---

## See Also

### Example

- `examples/auth_sample`

### API Reference

- [`AuthSupport`](../../reference/authentication-and-sessions/auth-support.md)
- [`PageAuthSupport`](../../reference/authentication-and-sessions/page-auth-support.md)
- [`ActionAuthSupport`](../../reference/authentication-and-sessions/action-auth-support.md)
- [`Session`](../../reference/authentication-and-sessions/session.md)
- [`SessionStore`](../../reference/authentication-and-sessions/session-store.md)
