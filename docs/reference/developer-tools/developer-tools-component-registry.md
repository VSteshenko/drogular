# `DeveloperToolsComponentRegistry`

**Namespace:** `drogular`  
**Header:** `<drogular/developer_tools/component_registry.hpp>`  
**Kind:** Registry class

## Purpose

`DeveloperToolsComponentRegistry` maps semantic Developer Tools component names to trusted browser-side JavaScript modules used by [`DiagnosticsPage`](diagnostics-page.md).

Inspection data and browser rendering are registered independently.

## Endpoint

```cpp
static constexpr const char* Path =
    "/__drogular/developer-tools/components";
```

When the diagnostics page is enabled, this path returns the registry as JSON.

## Registration Type

```cpp
struct DeveloperToolsComponentRegistration {
    std::string name;
    std::string module;
};
```

`name` is the semantic component name stored in an inspection section. `module` is the browser-importable module URL.

## Public API

### `add()`

```cpp
void add(
    std::string name,
    std::string module
);
```

Registers a component mapping.

- empty `name` throws `std::invalid_argument`;
- empty `module` throws `std::invalid_argument`;
- adding an existing name replaces its module in place.

Application code normally uses:

```cpp
app.developerToolsComponent(
    "example.system-info",
    "/assets/system-info.js"
);
```

### `find()`

```cpp
const DeveloperToolsComponentRegistration* find(
    std::string_view name
) const;
```

Returns a pointer to the matching registration or `nullptr`.

The pointer refers to storage owned by the registry and can be invalidated by later modifications to the underlying vector.

### `entries()`

```cpp
const std::vector<
    DeveloperToolsComponentRegistration
>& entries() const;
```

Returns the complete ordered registration list.

### `toJson()`

```cpp
Json::Value toJson() const;
```

Produces:

```json
{
  "components": [
    {
      "name": "example.system-info",
      "module": "/assets/system-info.js"
    }
  ]
}
```

## Browser Module Contract

The diagnostics client dynamically imports the registered `module` URL.

A module must export either:

```js
export function render(container, context) {
    // ...
}
```

or a default renderer function with the same signature.

`context` contains:

```text
id
title
component
data
```

If the component name is not registered, the module cannot be loaded, or it does not export a renderer function, the diagnostics page falls back to displaying the section data as JSON.

## Security Boundary

Modules registered here execute in the diagnostics page browser context. Treat module URLs as trusted application configuration rather than untrusted inspection data.

## Thread Safety

The registry has no internal synchronization. Register browser components during application startup.

## Related Types

- [`InspectionSection`](application-inspection.md#inspectionsection)
- [`DiagnosticsPage`](diagnostics-page.md)
- [`DeveloperToolsContributor`](developer-tools-contributor.md)
- [`App`](../application/app.md)
