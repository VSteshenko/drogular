# Authentication and Sessions

Drogular provides session-based primitives for establishing application identity and enforcing simple access requirements.

The framework does not define users, credentials, roles, permissions, or persistence. Applications build those concepts on top of `SessionStore`, `Session`, and the authentication support helpers.

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
     │
     ├── AuthSupport
     ├── PageAuthSupport
     └── ActionAuthSupport
```

## Types

- [`AuthSupport`](auth-support.md) — read authentication-related session values from render or action contexts.
- [`PageAuthSupport`](page-auth-support.md) — expose page-level authentication and access flags through `RenderContext`.
- [`ActionAuthSupport`](action-auth-support.md) — stop action execution by returning redirect results when session requirements are not met.
- [`Session`](session.md) — store string key/value data for one session.
- [`SessionStore`](session-store.md) — create, look up, remove, and clear in-memory sessions.

## Typical Flow

A login action authenticates application credentials, creates or reuses a session, stores identity data, and returns the session id as a cookie:

```cpp
auto session = context.session();

session->set("username", user->username);
session->set("role", user->role);

const auto sessionId =
    session->get("_id").value();

return drogular::ActionResult::redirect("/dashboard")
    .cookie("session_id", sessionId);
```

Subsequent requests use the `session_id` cookie to resolve the session from `SessionStore`.

## Important Semantics

`AuthSupport` and `ActionAuthSupport` currently use different authentication checks:

- `AuthSupport::isAuthenticated()` returns `true` only when the resolved session contains a `username` value.
- `ActionAuthSupport::requireAuthentication()` only requires an existing session and does not require `username`.

Applications should choose a consistent identity convention and use the appropriate helper deliberately.

## Storage and Lifetime

`SessionStore` is an in-memory store. Sessions are lost when the process stops and are not shared between application processes.

The current implementation does not provide:

- expiration or TTL;
- persistence;
- automatic cookie creation;
- automatic cookie invalidation;
- CSRF protection;
- role or permission models;
- internal synchronization for concurrent mutation.

## Cookbook

- Authentication & Authorization *(coming soon)*

## Related Reference

- [`ActionContext`](../actions/action-context.md)
- [`ActionResult`](../actions/action-result.md)
- [`RenderContext`](../rendering/render-context.md)
- [`ApplicationServices`](../dependency-injection/application-services.md)
