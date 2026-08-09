# `ApplicationProfile`

**Namespace:** `drogular`  
**Header:** `<drogular/application_profile.hpp>`  
**Kind:** Enumeration

## Purpose

`ApplicationProfile` identifies the runtime profile of an application.

The profile is stored by `App`. Selecting `Development` automatically enables Developer Tools unless they were explicitly disabled before the profile was applied.

---

## Synopsis

```cpp
enum class ApplicationProfile {
    Development,
    Testing,
    Production
};

constexpr const char* toString(
    ApplicationProfile profile
);
```

---

## Values

### `Development`

Development profile.

Applying this profile through `App::profile()` enables Developer Tools automatically unless `App::disableDeveloperTools()` was called first.

### `Testing`

Testing profile.

The current implementation does not automatically enable Developer Tools or change application options for this profile.

### `Production`

Production profile.

This is the default profile for a newly constructed `App`.

Developer Tools are not enabled automatically.

---

## `toString()`

```cpp
constexpr const char* toString(
    ApplicationProfile profile
);
```

Returns:

| Value | Result |
|---|---|
| `Development` | `"development"` |
| `Testing` | `"testing"` |
| `Production` | `"production"` |

The fallback return value is `"production"`.

---

## Example

```cpp
drogular::App app;

app.profile(
    drogular::ApplicationProfile::Development
);
```

Developer Tools are enabled as part of applying the Development profile.

To keep the Development profile without automatically registering Developer Tools, disable them first:

```cpp
drogular::App app;

app.disableDeveloperTools()
    .profile(drogular::ApplicationProfile::Development);
```

---

## Related Types

- [`App`](app.md)

## See Also

- [Developer Tools API Reference](../developer-tools/README.md)
- [Developer Tools example](../../../examples/developer_tools/README.md)
