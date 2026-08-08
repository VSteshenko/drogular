# `AuthSupport`

**Namespace:** `drogular`  
**Header:** `<drogular/auth_support.hpp>`  
**Kind:** Utility class with static methods

## Purpose

`AuthSupport` reads authentication-related values from the session associated with a `RenderContext` or `ActionContext`.

It does not define users, roles, permissions, or login logic.

## Public API

### `isAuthenticated()`

```cpp
static bool isAuthenticated(RenderContext& context);
static bool isAuthenticated(ActionContext& context);
```

Returns `true` when the current session contains the key `username`.

The implementation is equivalent to checking whether:

```cpp
sessionValue(context, "username").has_value()
```

A session may therefore exist while `isAuthenticated()` still returns `false`.

### `sessionValue()`

```cpp
static std::optional<std::string> sessionValue(
    RenderContext& context,
    const std::string& key
);

static std::optional<std::string> sessionValue(
    ActionContext& context,
    const std::string& key
);
```

Resolves the `session_id` request cookie, looks up the session in the registered [`SessionStore`](session-store.md), and returns the requested value.

Returns `std::nullopt` when:

- the `session_id` cookie is absent or empty;
- the session id is unknown;
- the key is not present in the session.

`SessionStore` is resolved with `requireService<SessionStore>()` after a session cookie is found. If the service is not registered, service resolution throws.

### `hasSessionValue()`

```cpp
static bool hasSessionValue(
    RenderContext& context,
    const std::string& key,
    const std::string& expectedValue
);

static bool hasSessionValue(
    ActionContext& context,
    const std::string& key,
    const std::string& expectedValue
);
```

Returns `true` only when the requested session value exists and exactly equals `expectedValue`.

## Example

```cpp
if (drogular::AuthSupport::isAuthenticated(context)) {
    const auto role =
        drogular::AuthSupport::sessionValue(
            context,
            "role"
        );
}
```

PortalDemo builds its application-specific `PortalAuthSupport` on top of these primitives rather than extending the framework authentication model.

## Behavior

`AuthSupport` performs a fresh store lookup for each call. It does not cache a session in the context.

Authentication is specifically tied to the `username` session key. Storing only `user_id`, for example, does not make `isAuthenticated()` return `true`.

## Thread Safety

`AuthSupport` itself has no mutable state. Thread-safety depends on [`SessionStore`](session-store.md) and [`Session`](session.md), which currently have no internal synchronization.

## Related Types

- [`Session`](session.md)
- [`SessionStore`](session-store.md)
- [`PageAuthSupport`](page-auth-support.md)
- [`ActionAuthSupport`](action-auth-support.md)
- [`RenderContext`](../rendering/render-context.md)
- [`ActionContext`](../actions/action-context.md)
