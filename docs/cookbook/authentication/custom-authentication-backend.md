# Custom Authentication Backend

## Problem

**Need to authenticate users against an existing identity system without coupling that system to Drogular?**

Drogular intentionally does not define a built-in user model, user database, or authentication-provider interface.

Instead, applications authenticate users with their own services and then establish a Drogular session containing the identity required by the rest of the application.

This makes the same page, action, and authorization model work with local databases, LDAP, Active Directory, OAuth2/OpenID Connect, SAML, REST services, or proprietary identity systems.

---

## Recommended Solution

Keep the authentication backend behind an application service.

That service verifies credentials or external identity assertions and returns an application-defined authenticated user. After authentication succeeds, the login action maps that identity into a fresh server-side session.

```text
User
    │
    ▼
Login Action
    │
    ▼
Application Authentication Service
    │
    ▼
Authenticated Identity
    │
    ▼
SessionStore / Session
    │
    ├── Protected Pages
    └── Protected Actions
```

Drogular does not require a special authentication interface. The application owns the authentication technology; Drogular owns the reusable session and endpoint-protection infrastructure.

---

## Why Drogular Does Not Own Your Users

One of Drogular's core architectural decisions is that the framework does not own your users.

Many web frameworks provide a complete authentication stack, including user entities, database schema, password storage, and authentication services. While convenient for new applications, that also couples the framework to a particular user model.

Drogular intentionally takes a different approach.

The framework assumes only one thing:

> At some point, the application establishes the identity of the current user.

Everything before that moment belongs to the application. Once identity has been established, Drogular can manage the server-side session and provide reusable protection primitives for pages and actions.

```text
Application                                Drogular

Verify credentials
Query SQL / LDAP
Call an identity provider
Map external identity
        │
        ▼
Authenticated Identity
        │
        └─────────────────────────────────► Session
                                           Page protection
                                           Action protection
                                           Authorization helpers
```

This boundary keeps authentication technology independent from the rest of the application.

---

## Implement an Application Authentication Service

The `auth_sample` example uses a small application service:

```cpp
class AuthService {
public:
    std::optional<AuthUser> authenticate(
        const std::string& username,
        const std::string& password
    ) const;
};
```

Its result is also application-defined:

```cpp
struct AuthUser {
    int id = 0;
    std::string username;
    std::string role;
};
```

Neither type is part of Drogular's authentication API.

A different project may use a different service interface, return a richer domain object, or authenticate without passwords at all.

For example, an OpenID Connect callback might validate an authorization response and then return the same application-level `AuthUser` used by the rest of the application.

---

## Register the Backend Through Dependency Injection

Authentication services are ordinary application services and can be registered through Drogular DI:

```cpp
app.services().add<AuthService>(
    drogular::ServiceLifetime::Singleton
);
```

An action resolves the service exactly like any other dependency:

```cpp
auto authService =
    context.requireService<AuthService>();
```

No authentication-specific registration contract is required.

That means an application can replace one implementation with another without changing Drogular's session or endpoint-protection APIs.

---

## Authenticate Before Creating Session State

The backend should complete authentication before the application creates authenticated session state.

In `auth_sample`, the login action first validates input and then calls `AuthService::authenticate()`:

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

Only the successful branch continues into session rotation and identity storage.

This keeps invalid credentials from creating authenticated session state and keeps the framework independent from credential verification details.

---

## Map External Identity Into a Minimal Session Principal

External identity systems often return far more information than protected requests need.

For example, an LDAP directory may expose:

```text
LDAP entry
    │
    ├── uid
    ├── displayName
    ├── mail
    ├── department
    ├── groups
    └── many other attributes
```

Do not copy the entire external object into the session.

Map it to a minimal application identity instead:

```text
External Identity
       │
       ▼
Application AuthUser
       │
       ▼
Session
    user_id
    username
    role
```

`auth_sample` stores:

```cpp
session->set("user_id", std::to_string(user->id));
session->set("username", user->username);
session->set("role", user->role);
```

The framework authentication baseline uses `username`, while the sample's application-level `AuthSession` additionally requires `role` before constructing a complete current user.

Applications may choose a different principal contract, such as `user_id`, `tenant_id`, permissions, or another trusted identity marker.

---

## Integrate Different Identity Sources

Because Drogular depends only on the resulting authenticated session, the authentication source can change without changing protected pages or actions.

### SQL or another application database

```text
Login Action
    │
    ▼
AuthService
    │
    ▼
Users Repository
    │
    ▼
Authenticated Identity
```

The application owns password hashing, credential comparison, account state, and user lookup.

### LDAP or Active Directory

```text
Login Action
    │
    ▼
Directory Authentication Service
    │
    ▼
LDAP / Active Directory
    │
    ▼
Mapped Application Identity
```

Directory attributes and groups can be translated into the application's own role or permission model before session creation.

### OAuth2 or OpenID Connect

```text
Browser
    │
    ▼
External Identity Provider
    │
    ▼
Application Callback
    │
    ▼
Validate Provider Response
    │
    ▼
Application Identity
    │
    ▼
Drogular Session
```

The external protocol establishes identity. The application then creates the same server-side session that a local login would create.

Drogular pages and actions do not need to know whether the identity originated from a password, LDAP bind, OAuth2 flow, or another mechanism.

### External REST authentication service

```text
Login Action
    │
    ▼
Application AuthService
    │
    ▼
Remote Authentication API
    │
    ▼
Application Identity
```

The remote response should still be mapped to an application-owned identity contract before session state is established.

---

## Keep Provider-Specific Data Out of Protected Endpoints

Protected pages and actions should depend on the authenticated application identity, not directly on the authentication backend.

Avoid designs such as:

```text
DashboardPage ──► LDAP client
AdminAction   ──► OAuth provider
ReportsPage   ──► authentication REST API
```

Prefer:

```text
Authentication Backend
        │
        ▼
Application Identity
        │
        ▼
Session
        │
        ├── DashboardPage
        ├── AdminAction
        └── ReportsPage
```

This keeps authentication work at the authentication boundary and makes protected endpoints stable even when the backend changes.

If fresh domain data is needed later, load it through normal application services using a stable identity such as `user_id` rather than repeating authentication.

---

## Changing Authentication Backends

A useful consequence of this architecture is that authentication technology can evolve independently.

```text
Local database
      │
      ▼
    LDAP
      │
      ▼
OpenID Connect
      │
      ▼
Corporate identity platform
```

If each backend produces the same application identity/session contract, the following code can remain unchanged:

- protected pages;
- protected actions;
- routing;
- page/action authorization helpers;
- most application policy code.

Only the authentication boundary and, when necessary, identity mapping need to change.

---

## Identity Design

A session should represent the authenticated principal, not become a copy of the user database.

Good candidates include:

- stable user identifier;
- username or login name;
- role when role checks are intentionally session-based;
- tenant identifier for a multi-tenant application;
- other small trusted values required on most protected requests.

Avoid storing:

- plaintext passwords;
- password hashes;
- entire ORM entities;
- LDAP objects;
- large user profiles;
- mutable business data that should be loaded from its source of truth;
- external access or refresh tokens unless the application genuinely needs them after login and has an appropriate security policy for them.

Keep the principal minimal and explicit.

---

## Session Creation Remains the Same

The backend changes how identity is established, not how Drogular session state works.

After successful authentication, use the normal login lifecycle:

1. remove any old session referenced by the incoming cookie;
2. create a fresh session;
3. store the trusted application identity;
4. send the new session identifier cookie.

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

Session rotation and cookie policy are covered in detail in [Login & Logout](login-logout.md) and [Session Management](session-management.md).

---

## Authorization Remains an Application Policy

Changing the authentication backend does not change the distinction between authentication and authorization.

The backend answers:

> Who is the user?

Protected endpoints and application policy answer:

> May this user perform this operation?

For simple role checks, an application may map a trusted role into the session and use Drogular's session-value helpers.

For richer policies, keep the authenticated identity stable and move authorization decisions into an application authorization service.

Do not embed provider-specific concepts such as LDAP groups or OAuth claims directly throughout pages and actions unless they are intentionally part of your application authorization model.

---

## Best Practices

When integrating a custom authentication backend:

- keep credential verification in an application service;
- map external identity into an application-owned principal;
- create authenticated session state only after authentication succeeds;
- rotate the session identifier after successful login;
- store only the minimal trusted identity required for future requests;
- keep passwords and unnecessary provider data out of sessions;
- keep protected pages and actions independent from the authentication provider;
- separate authentication from authorization;
- treat missing or incomplete identity state as unauthenticated;
- use HTTPS and `Secure` cookies in production;
- define explicit lifecycle rules for external tokens if the application must retain them.

---

## Architectural Principle

A concise way to remember the model is:

> Authentication belongs to the application.  
> Session management belongs to the framework.  
> Authorization belongs to the protected endpoint.

The authentication technology can change while the rest of the Drogular application continues to work against the same authenticated session contract.

---

## See Also

- [Authentication Overview](overview.md)
- [Login & Logout](login-logout.md)
- [Protecting Pages](protecting-pages.md)
- [Protecting Actions](protecting-actions.md)
- [Role-Based Authorization](role-based-authorization.md)
- [Session Management](session-management.md)
- [Authentication & Sessions API Reference](../../reference/authentication-and-sessions/README.md)
- [Dependency Injection API Reference](../../reference/dependency-injection/README.md)
