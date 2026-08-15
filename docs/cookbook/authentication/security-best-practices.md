# Security Best Practices

## Problem

**Need a concise security checklist for authentication code built with Drogular?**

Authentication security does not come from one API call or one framework feature.

It emerges from consistently applying a small set of rules across login, session management, pages, actions, authorization, and deployment.

This guide summarizes the practices demonstrated throughout the Authentication Cookbook and in `examples/auth_sample`.

---

## Recommended Model

Keep the security flow explicit:

```text
Authentication
      │
      ▼
Authenticated Identity
      │
      ▼
Server-Side Session
      │
      ▼
Authorization
      │
      ▼
Protected Endpoint
```

Each stage has one responsibility:

- the application authenticates the user;
- the session carries trusted server-side identity state between requests;
- each protected endpoint enforces its own authorization requirements.

Do not collapse these responsibilities into one implicit check.

---

## Authenticate Once

Authenticate credentials only when establishing a login.

After successful authentication, subsequent requests should use the authenticated session rather than repeatedly validating the user's credentials.

```text
Login
  │
  ▼
Authenticate
  │
  ▼
Create authenticated session
  │
  ▼
Protected requests use session identity
```

This keeps credential handling isolated from normal request processing and gives the rest of the application a stable identity contract.

---

## Separate Authentication and Authorization

Authentication answers:

> Who is the current user?

Authorization answers:

> Is this user allowed to perform this operation?

Always establish a valid identity before checking roles, permissions, ownership, or other application policy.

For example, an admin endpoint should first require an authenticated user and only then require the `admin` role.

This distinction also makes failures meaningful:

- unauthenticated users need to log in;
- authenticated but unauthorized users must be denied access.

---

## Protect Every Endpoint

Every externally reachable entry point is its own security boundary.

Protecting a page does not protect the action submitted from that page. Hiding a button does not protect its POST endpoint. Protecting an action does not automatically protect another API route that performs the same operation.

```text
Browser
   │
   ├──► Page   ──► page requirement
   │
   ├──► Action ──► action requirement
   │
   └──► API    ──► API-specific requirement
```

Apply authentication and authorization at the endpoint that performs the protected work.

For pages, stop before loading protected data:

```cpp
if (!AuthSession::requireCurrentUser(context)) {
    return;
}

// Protected page work starts here.
```

For actions, return the denial result immediately:

```cpp
if (auto denied =
        drogular::ActionAuthSupport::requireAuthentication(context)) {
    return *denied;
}

// Protected mutation starts here.
```

---

## Fail Closed

Missing, incomplete, malformed, or inconsistent authentication state must deny access.

Do not guess identity from partial session values and do not continue protected work after a failed requirement.

The `auth_sample` demonstrates this with an application-level current-user contract: a session that contains only part of the expected identity is not treated as a valid authenticated principal.

A useful rule is:

```text
Identity requirement satisfied? ── no ──► stop
             │
            yes
             │
             ▼
      protected work
```

Fail-closed behavior turns unexpected session state into denial rather than privilege.

---

## Rotate Sessions After Login

Never promote an existing anonymous session identifier into an authenticated session.

After credentials have been successfully verified:

1. remove the old server-side session, if one exists;
2. create a fresh session;
3. store the authenticated identity;
4. send the new session identifier to the client.

```text
Existing session
      │
      ▼
Successful authentication
      │
      ▼
Destroy old session
      │
      ▼
Create fresh authenticated session
```

This prevents session fixation and gives authenticated state a clear lifecycle boundary.

Do not rotate sessions for failed login attempts. Authentication should succeed before authenticated session state is created.

See [Login & Logout](login-logout.md) for the complete flow.

---

## Keep Sessions Small

Store only trusted identity information that future requests actually need.

Typical values include:

- stable user identifier;
- username or another display identity;
- role or compact authorization claims;
- tenant identifier when required by the application.

Avoid putting the following in the session:

- passwords;
- password hashes;
- complete ORM entities;
- large user profiles;
- provider-specific objects;
- unrelated application cache data;
- external access tokens unless the application genuinely needs them between requests.

A session represents authenticated identity and small session-scoped state. It should not become a general-purpose cache.

---

## Trust Server-Side State

Make authorization decisions from trusted server-side state.

Do not trust security claims supplied by the browser, including:

- hidden form fields;
- JavaScript variables;
- browser storage;
- query parameters;
- UI visibility;
- client-side role checks.

The client can be modified by the user.

With Drogular's default session model, the browser carries the session identifier while identity values remain in the server-side `SessionStore`.

That server-side session is the appropriate source for authentication and authorization helpers.

---

## Keep Security Decisions Out of Templates

Templates should display the result of a security decision, not make the decision themselves.

A page can expose render state such as:

```cpp
context.set("loginRequired", true);
```

and the template may render the corresponding UI:

```html
{% if loginRequired %}
    <p>Please sign in.</p>
{% endif %}
```

The template must not be the only place that checks whether protected content or operations are allowed.

Security belongs in C++ control flow before protected data is loaded or mutations are executed.

---

## Keep Authentication Services Independent

Authentication backends should remain ordinary application services.

Register them through dependency injection and keep them independent from pages, templates, and routing details.

```cpp
app.services().add<AuthService>(
    drogular::ServiceLifetime::Singleton
);
```

This makes authentication implementations easier to:

- replace;
- test;
- reuse;
- integrate with SQL, LDAP, OpenID Connect, or another identity source.

Protected pages and actions should depend on the authenticated session contract, not on the technology that originally verified the user.

See [Custom Authentication Backend](custom-authentication-backend.md).

---

## Use Secure Cookie Settings

Production applications should serve authentication traffic over HTTPS and configure authentication cookies accordingly.

For a session identifier cookie, normally consider:

- `HttpOnly` to prevent normal JavaScript access;
- `Secure` when the application is served over HTTPS;
- an appropriate `SameSite` policy;
- a narrowly scoped path and domain where appropriate.

`auth_sample` intentionally uses `Secure=false` because the example runs locally over plain HTTP. Do not copy that development setting unchanged into an HTTPS production deployment.

Cookie policy is application and deployment specific. Review it together with CSRF protections and any cross-site authentication flows used by the application.

---

## Complete Logout on Both Sides

Logout has two responsibilities:

1. remove the server-side session;
2. expire the client-side session identifier cookie.

Doing only one leaves unnecessary state behind.

```text
Logout
   │
   ├──► SessionStore::remove(...)
   │
   └──► expire session_id cookie
```

After logout, a request carrying the old identifier must no longer recover an authenticated session.

---

## Define Session Lifetime Explicitly

The default `SessionStore` is an in-memory, process-local store. Applications that need expiration, cleanup, persistence, shared multi-process sessions, or stronger operational controls should define those requirements explicitly.

Do not assume that authentication session lifetime is handled automatically merely because session storage exists.

Production systems may need an application-specific or external session backend with policies for:

- inactivity timeout;
- absolute lifetime;
- revocation;
- cleanup;
- shared storage across processes or hosts;
- operational monitoring.

See [Session Management](session-management.md) for the default store's behavior and limitations.

---

## Keep Authorization Policy Centralized

Simple checks can use Drogular's session-value helpers directly:

```cpp
if (!drogular::PageAuthSupport::requireSessionValue(
        context,
        "role",
        "admin")) {
    return;
}
```

As policy grows, avoid scattering raw role strings and provider-specific claims throughout the application.

Prefer application helpers or an authorization service that expresses business policy in one place.

For example:

```text
Session identity
      │
      ▼
Application authorization policy
      │
      ▼
canManageUsers(user)
canEditProject(user, project)
canViewAuditLog(user)
```

Drogular provides protection primitives; the application owns the meaning of roles and permissions.

See [Role-Based Authorization](role-based-authorization.md).

---

## Test Denial Paths as Carefully as Success Paths

Authentication and authorization tests should cover more than successful requests.

Recommended scenarios include:

- anonymous access to protected pages;
- anonymous access to protected actions;
- authenticated user access;
- administrator access;
- wrong role;
- missing session;
- incomplete session;
- malformed or stale identity state;
- successful login;
- failed login;
- session rotation;
- logout;
- repeated access after logout;
- forbidden mutation;
- successful mutation.

For commands, verify both the response and the absence of unauthorized side effects.

For example, a forbidden delete action should be tested not only for its redirect or denial result, but also for the fact that the underlying object still exists.

Security tests are strongest when they verify the boundary itself rather than only the presentation shown to the user.

---

## Security Checklist

Before treating an authentication flow as production-ready, verify that:

- authentication and authorization are separate;
- protected pages check access before loading protected data;
- protected actions check access before validation or mutation where appropriate;
- every security helper's denial result is respected immediately;
- incomplete session identity fails closed;
- successful login rotates the session identifier;
- failed login does not create authenticated session state;
- logout removes the server-side session and expires the cookie;
- sessions contain only minimal trusted identity state;
- authorization does not trust browser-controlled values;
- templates do not form the security boundary;
- authentication services are isolated behind application services;
- HTTPS and production cookie attributes are enabled in production;
- session lifetime and deployment topology have explicit policies;
- denial paths and unauthorized side effects are covered by tests.

---

## Summary

Drogular intentionally keeps authentication infrastructure small and composable.

The application authenticates users. The framework manages the authenticated session. Protected endpoints enforce authorization.

The strongest security model comes from applying these boundaries consistently rather than relying on any single helper or feature.

> **Security is not achieved by a single feature.**
>
> **It emerges from consistently applying simple rules throughout the application.**

The `auth_sample` demonstrates this principle in practice. Its security comes from the combination of explicit page and action contracts, fail-closed behavior, session rotation, minimal server-side identity, independent endpoint protection, and clear separation between authentication and authorization.

Individually, each rule is simple. Together, they create a model that is easier to understand, easier to test, and more resistant to implementation mistakes.

---

## See Also

- [Authentication Overview](overview.md)
- [Login & Logout](login-logout.md)
- [Protecting Pages](protecting-pages.md)
- [Protecting Actions](protecting-actions.md)
- [Role-Based Authorization](role-based-authorization.md)
- [Session Management](session-management.md)
- [Custom Authentication Backend](custom-authentication-backend.md)
- [Authentication & Sessions API Reference](../../reference/authentication-and-sessions/README.md)
