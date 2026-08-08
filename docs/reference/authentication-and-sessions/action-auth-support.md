# `ActionAuthSupport`

**Namespace:** `drogular`  
**Header:** `<drogular/action_auth_support.hpp>`  
**Kind:** Utility class with static methods

## Purpose

`ActionAuthSupport` adapts session checks to action handling by returning an `ActionResult` redirect when a requirement fails.

A successful check returns `std::nullopt`, allowing the action to continue.

## Public API

### `requireAuthentication()`

```cpp
static std::optional<ActionResult> requireAuthentication(
    ActionContext& context,
    const std::string& redirectPath = "/login"
);
```

Returns a redirect result when `ActionContext::existingSession()` returns `nullptr`.

Returns `std::nullopt` when any existing session is present.

> **Important:** this differs from `AuthSupport::isAuthenticated()`. `ActionAuthSupport::requireAuthentication()` does not require the session to contain `username`.

### `requireSessionValue()`

```cpp
static std::optional<ActionResult> requireSessionValue(
    ActionContext& context,
    const std::string& key,
    const std::string& expectedValue,
    const std::string& redirectPath
);
```

Returns a redirect when:

- no existing session is available;
- the key is missing;
- the value differs from `expectedValue`.

Returns `std::nullopt` when the requirement succeeds.

## Example

```cpp
drogular::ActionResult handle(
    drogular::ActionContext& context
) override {
    if (const auto denied =
            drogular::ActionAuthSupport::requireAuthentication(context)) {
        return *denied;
    }

    if (const auto denied =
            drogular::ActionAuthSupport::requireSessionValue(
                context,
                "role",
                "admin",
                "/dashboard"
            )) {
        return *denied;
    }

    // Protected command.
    return drogular::ActionResult::redirect("/admin");
}
```

## Behavior

The helper returns an `ActionResult`; it does not send the response itself. The action must return the result.

The default authentication failure location is `/login`. Session-value checks require an explicit redirect path.

## Related Types

- [`AuthSupport`](auth-support.md)
- [`ActionContext`](../actions/action-context.md)
- [`ActionResult`](../actions/action-result.md)
- [`Session`](session.md)
