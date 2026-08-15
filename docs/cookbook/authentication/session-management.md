# Session Management

## Problem

**Need to keep authenticated state between requests without putting application identity data in the browser?**

Drogular provides a small server-side session model through `Session` and `SessionStore`.

The browser carries a session identifier. The actual session values remain in the application process.

```text
Browser
   │
   │ session_id cookie
   ▼
Application
   │
   ▼
SessionStore
   │
   └── session_id → Session
                       │
                       ├── user_id
                       ├── username
                       └── role
```

The complete authentication example is available in `examples/auth_sample`.

---

## Recommended Model

Treat the cookie and the server-side session as two different things:

- the cookie identifies a session;
- `SessionStore` resolves that identifier;
- `Session` contains the application state associated with it.

Do not put authentication state such as roles or user records directly in the session cookie when using this model.

A typical authenticated lifecycle is:

```text
Successful login
      │
      ▼
Create fresh Session
      │
      ▼
Store trusted identity values
      │
      ▼
Send session_id cookie
      │
      ▼
Subsequent request
      │
      ▼
Resolve Session from SessionStore
      │
      ▼
Read authentication / authorization state
      │
      ▼
Logout
      │
      ├── remove server-side Session
      └── expire session_id cookie
```

The session lifecycle is application-visible. Drogular does not automatically issue or expire authentication cookies for you.

---

## Register `SessionStore`

Authentication helpers resolve `SessionStore` through dependency injection, so applications normally register one shared store:

```cpp
app.services().add<drogular::SessionStore>(
    drogular::ServiceLifetime::Singleton
);
```

A singleton lifetime is important for the default in-memory implementation. Creating separate stores per request would make sessions invisible to later requests.

The default store is thread-safe for its own map operations and can be shared by concurrent request-processing threads.

---

## Creating a Session

Create a session explicitly through the store when you need full control over its lifecycle:

```cpp
auto store =
    context.requireService<drogular::SessionStore>();

auto session = store->create();
```

`SessionStore::create()`:

1. generates an identifier;
2. creates a `Session`;
3. stores the identifier under the internal `_id` key;
4. inserts the session into the store;
5. returns a `std::shared_ptr<Session>`.

The identifier can be read from the created session:

```cpp
const auto sessionId =
    session->get("_id").value();
```

`_id` is framework-managed by convention, but `Session` itself does not reserve or protect the key. Application code should treat it as read-only.

---

## `session()` vs `existingSession()`

`ActionContext` exposes two session access patterns.

### Read only if a session already exists

Use `existingSession()` when the operation must not create a new session:

```cpp
const auto session = context.existingSession();

if (session == nullptr) {
    return drogular::ActionResult::redirect("/login");
}
```

`existingSession()`:

- reads the incoming `session_id` cookie;
- resolves the configured `SessionStore` when available;
- returns the matching session;
- returns `nullptr` when no usable session exists.

It does not create state.

This is usually the safer choice for authentication checks, logout logic, and other read-only session operations.

### Reuse or create

Use `session()` when the application intentionally wants a session and creating one is acceptable:

```cpp
auto session = context.session();
```

`session()` first tries `existingSession()`. If no existing session is found, it creates a new one through `SessionStore`.

That distinction matters at security boundaries. A failed authentication or authorization check should normally not create a fresh anonymous session as a side effect.

Also note that `session()` only creates the server-side object. It does **not** automatically attach the generated identifier to the response. Application code must issue the cookie explicitly.

---

## Storing Session Values

`Session` stores string key/value pairs:

```cpp
session->set("user_id", "42");
session->set("username", "alice");
session->set("role", "admin");
```

Read values with `get()`:

```cpp
const auto username = session->get("username");

if (!username.has_value()) {
    // Treat the session as incomplete or unauthenticated.
}
```

Other operations are available when needed:

```cpp
session->has("role");
session->remove("role");
session->clear();
```

Individual operations on a `Session` are synchronized internally and may be called concurrently.

A sequence of several operations is not automatically atomic. If application correctness depends on a read-modify-write sequence being indivisible, coordinate that sequence at the application level.

---

## What to Store

Keep authenticated session state small and intentional.

Good candidates include:

- stable user identifier;
- username or display identity needed by the application;
- role or compact authorization claims;
- small pieces of request-independent state that genuinely belong to the session.

For example, `auth_sample` stores:

```cpp
session->set(
    "user_id",
    std::to_string(user->id)
);

session->set(
    "username",
    user->username
);

session->set(
    "role",
    user->role
);
```

Avoid using the session as a general object cache.

Do not store:

- plaintext passwords;
- password hashes merely for convenience;
- large domain objects;
- request-local data;
- values that must always reflect the current database state;
- sensitive external credentials unless the application has a deliberate protection and lifecycle strategy for them.

The more authorization state is copied into a session, the more carefully the application must decide when that state becomes stale and when the user must be revalidated.

---

## Authentication State Is an Application Contract

Drogular's built-in `AuthSupport::isAuthenticated()` uses the presence of `username` as its baseline authentication convention.

Applications may require a stronger identity contract.

`auth_sample` requires both `username` and `role` before constructing an `AuthUser`:

```cpp
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
```

This is intentionally fail-closed: a session that exists but is missing required identity values is not treated as a valid authenticated identity.

Session existence alone is therefore **not** proof of authentication.

---

## Rotate the Session on Login

A successful login should establish authentication on a fresh session identifier.

`auth_sample` removes any session referenced by the incoming cookie:

```cpp
if (const auto existingSessionId =
        context.cookie("session_id")) {
    sessionStore->remove(*existingSessionId);
}
```

It then creates a new session and stores the authenticated identity:

```cpp
auto session = sessionStore->create();

session->set("username", user->username);
session->set("role", user->role);
```

Finally, it sends the new identifier to the browser.

```text
Old session_id
      │
      ▼
Credentials verified
      │
      ▼
Remove old mapping
      │
      ▼
Create new Session
      │
      ▼
Store authenticated identity
      │
      ▼
Send new session_id
```

Do not promote an already supplied anonymous session identifier into an authenticated session. Rotating the identifier after successful authentication is the important defense against session fixation in this flow.

See [Login & Logout](login-logout.md) for the complete action implementation.

---

## Issue the Cookie Explicitly

`SessionStore` manages server-side sessions. It does not manage browser cookies.

After creating a session, return the identifier explicitly:

```cpp
const auto sessionId =
    session->get("_id").value();

return drogular::ActionResult::redirect("/dashboard")
    .cookie(
        "session_id",
        sessionId,
        drogular::CookieOptions{
            .httpOnly = true,
            .secure = false,
            .sameSite = drogular::CookieSameSite::Lax
        }
    );
```

`auth_sample` uses `secure = false` because the local example runs over HTTP.

For production HTTPS deployments, session cookies should normally use `Secure=true` so browsers do not send them over plain HTTP.

`HttpOnly` prevents ordinary browser JavaScript from reading the cookie. `SameSite=Lax` provides a useful baseline against cross-site request contexts, although applications should still evaluate their CSRF requirements independently.

---

## Resolve Existing Sessions

Framework authentication helpers resolve the request cookie against `SessionStore` and fail closed when no matching session exists.

Conceptually:

```text
session_id cookie
      │
      ▼
SessionStore::get(id)
      │
      ├── missing ──► unauthenticated
      │
      ▼
Session
      │
      ▼
required identity / role value
```

An unknown, expired-by-application-policy, removed, or otherwise missing session identifier must not grant access merely because the browser still sends the cookie.

This is why the server-side store remains the source of truth.

---

## Logout Completely

Logout has two independent sides:

1. invalidate the server-side session;
2. expire the browser cookie.

`auth_sample` removes the mapping when a cookie is present:

```cpp
const auto sessionId =
    context.cookie("session_id");

if (sessionId.has_value()) {
    auto sessionStore =
        context.requireService<drogular::SessionStore>();

    sessionStore->remove(*sessionId);
}
```

Then it expires the cookie:

```cpp
return drogular::ActionResult::redirect("/login")
    .cookie(
        "session_id",
        "",
        drogular::CookieOptions{
            .httpOnly = true,
            .secure = false,
            .sameSite = drogular::CookieSameSite::Lax,
            .maxAge = 0
        }
    );
```

Clearing only the cookie leaves the server-side session mapping alive.

Removing only the server-side session leaves the browser repeatedly sending a stale identifier.

A complete logout does both.

---

## Removing a Session vs Destroying the Object

`SessionStore::remove(id)` removes the store mapping:

```cpp
store->remove(sessionId);
```

This makes future lookups by that identifier fail.

However, sessions are held through `std::shared_ptr`. Existing code that already holds a `shared_ptr<Session>` may keep that object alive after it has been removed from the store.

Therefore, session invalidation means:

> the identifier is no longer accepted by the store for future requests.

It does not guarantee that every in-process reference to the object is immediately destroyed.

Request handlers should avoid retaining session objects beyond the work that needs them.

---

## Default Store Limitations

The built-in `SessionStore` is deliberately small. It is useful for examples, tests, and applications whose deployment model fits its constraints.

The current implementation is:

- process-local;
- memory-only;
- non-persistent;
- thread-safe for individual store operations;
- without built-in expiration;
- without idle-timeout cleanup;
- without capacity limits;
- without distributed sharing between application processes.

Its current identifier generator uses a thread-local `std::mt19937_64` value encoded as hexadecimal text and retries collisions already present in the store.

That is an implementation detail, **not a cryptographic session-token guarantee**.

Applications with stronger production requirements should treat session storage and identifier generation as security infrastructure and use an appropriate production-grade design rather than assuming the default in-memory store provides those guarantees.

Typical additional requirements may include:

- cryptographically strong opaque identifiers;
- absolute expiration;
- idle expiration;
- periodic cleanup;
- revocation across application instances;
- shared storage for horizontally scaled deployments;
- persistence or deliberate non-persistence;
- operational limits and monitoring.

---

## Multi-Process Deployments

Because the default store lives inside one application process, two Drogular processes do not automatically share sessions.

```text
Browser
   │ session_id = abc
   │
   ├────────► Process A ──► SessionStore A contains abc
   │
   └────────► Process B ──► SessionStore B does not contain abc
```

If traffic may reach multiple processes, use a deployment strategy that preserves session affinity or introduce an application/session backend designed for shared state.

Do not assume process-local sessions become distributed merely because `SessionStore` is registered as a singleton. The singleton lifetime is only within one application service container/process.

---

## Session Expiration Is Application Policy

The default `Session` and `SessionStore` do not track creation time, last-access time, or expiration.

That means the application must decide what session lifetime means for its deployment.

Possible policies include:

- expire on browser close;
- fixed maximum lifetime;
- idle timeout;
- forced re-authentication for sensitive operations;
- explicit revocation after a role or account change.

If a role or permission stored in the session changes in the source of truth, the existing session does not update itself automatically.

Security-sensitive applications should define when authorization claims are refreshed or invalidated.

---

## Thread Safety

Both default session types synchronize their internal containers.

`Session` protects individual operations such as:

```cpp
set()
get()
has()
remove()
clear()
```

`SessionStore` protects individual operations such as:

```cpp
create()
get()
contains()
remove()
clear()
```

This makes ordinary concurrent access safe at the container-operation level.

It does not make multi-step application transactions atomic. For example:

```cpp
const auto value = session->get("counter");
// another thread may change the session here
session->set("counter", nextValue);
```

If a multi-step mutation must be atomic, add application-level coordination or model the state differently.

---

## Testing Session Behavior

Authentication tests should verify the session lifecycle itself, not only redirects.

### Successful login

Verify that:

- a new cookie is returned;
- the cookie identifies a stored session;
- the expected identity values exist in that session.

```cpp
const auto session =
    store->get(result.cookies()[0].value);

ASSERT_NE(session, nullptr);
EXPECT_EQ(
    session->get("username").value(),
    "admin"
);
```

### Session rotation

Start with an existing session and submit valid credentials.

Verify that:

- the returned identifier differs from the old one;
- the old mapping is gone;
- the new mapping exists;
- the new session contains the authenticated identity.

```cpp
EXPECT_NE(newSessionId, oldSessionId);
EXPECT_FALSE(store->contains(oldSessionId));
EXPECT_TRUE(store->contains(newSessionId));
```

### Logout

Verify both sides of logout:

```cpp
EXPECT_FALSE(store->contains(sessionId));
EXPECT_EQ(result.cookies()[0].value, "");
ASSERT_TRUE(result.cookies()[0].maxAge.has_value());
EXPECT_EQ(*result.cookies()[0].maxAge, 0);
```

### Invalid or incomplete sessions

Also cover requests where:

- no session cookie exists;
- the cookie references an unknown session;
- required identity values are missing;
- authorization state is missing or unexpected;
- a previously valid session has been removed.

These cases should fail closed.

---

## Common Mistakes

### Treating the cookie as the authentication state

The cookie is only an identifier in this session model. Resolve it against the server-side store before trusting any identity state.

### Creating sessions during authorization checks

Do not call session-creating APIs merely to check whether a user is authenticated. Prefer existing-session/auth helpers for read-only security decisions.

### Reusing the pre-login identifier

Do not keep an anonymous session identifier after successful authentication. Rotate it.

### Clearing only the browser cookie on logout

Remove the server-side session as well.

### Storing too much state

Sessions are not domain-object caches. Keep identity and authorization state compact.

### Assuming the default store expires sessions

It does not. Define expiration and cleanup explicitly when your deployment requires them.

### Assuming singleton means distributed

A DI singleton is process-local. Multiple application processes still have separate default stores.

---

## Summary

A secure session flow in Drogular follows a small set of rules:

1. register one appropriate `SessionStore` for the application;
2. keep authoritative session state on the server;
3. create a fresh session after successful authentication;
4. store only the identity/authorization values the application actually needs;
5. send only the opaque session identifier to the browser;
6. use existing-session lookups for checks that must not create state;
7. fail closed when a session or required identity value is missing;
8. remove server-side state and expire the cookie on logout;
9. define expiration, distribution, and stronger identifier requirements explicitly for production deployments.

---

## See Also

- [Authentication Overview](overview.md)
- [Login & Logout](login-logout.md)
- [Protecting Pages](protecting-pages.md)
- [Protecting Actions](protecting-actions.md)
- [Role-Based Authorization](role-based-authorization.md)
- [API Reference — `Session`](../../reference/authentication-and-sessions/session.md)
- [API Reference — `SessionStore`](../../reference/authentication-and-sessions/session-store.md)
- [API Reference — `AuthSupport`](../../reference/authentication-and-sessions/auth-support.md)
- [API Reference — `ActionContext`](../../reference/actions/action-context.md)
