# `PageAuthSupport`

**Namespace:** `drogular`  
**Header:** `<drogular/page_auth_support.hpp>`  
**Kind:** Utility class with static methods

## Purpose

`PageAuthSupport` adapts session checks to page rendering by setting standard boolean values in `RenderContext` and returning whether rendering may continue.

It does not redirect the request itself.

## Public API

### `requireAuthentication()`

```cpp
static bool requireAuthentication(
    RenderContext& context
);
```

Calls `AuthSupport::isAuthenticated(context)` and writes:

```text
loginRequired = !authenticated
```

Returns `true` when authentication succeeds.

Because [`AuthSupport::isAuthenticated()`](auth-support.md) requires a `username` session value, a session without `username` is treated as unauthenticated.

### `requireSessionValue()`

```cpp
static bool requireSessionValue(
    RenderContext& context,
    const std::string& key,
    const std::string& expectedValue
);
```

Checks a session value and writes:

```text
accessDenied = !allowed
```

Returns `true` when the value exists and exactly matches `expectedValue`.

## Example

PortalDemo guards pages during `onInit()`:

```cpp
void onInit(
    drogular::RenderContext& context
) override {
    if (!drogular::PageAuthSupport::requireAuthentication(context)) {
        return;
    }

    if (!drogular::PageAuthSupport::requireSessionValue(
            context,
            "role",
            "admin"
        )) {
        return;
    }

    // Populate protected page data.
}
```

Templates can then react to `loginRequired` or `accessDenied`.

## Behavior

The helpers only set render values and return booleans. They do not:

- stop `Page::render()` automatically;
- generate a redirect;
- select a login page;
- define roles or permissions.

The page must return early or otherwise react to the returned value.

## Related Types

- [`AuthSupport`](auth-support.md)
- [`RenderContext`](../rendering/render-context.md)
- [`Page`](../pages/page.md)
