# `diagnostics_resources`

**Namespace:** `drogular::diagnostics_resources`  
**Header:** `<drogular/developer_tools/diagnostics_resources.hpp>`  
**Kind:** Resource functions

## Purpose

`diagnostics_resources` exposes the embedded assets used by the built-in [`DiagnosticsPage`](diagnostics-page.md).

## Public API

```cpp
std::string_view pageHtml();
std::string_view stylesheet();
std::string_view script();
```

### `pageHtml()`

Returns the complete diagnostics application shell.

The page references:

```text
/__drogular/inspection
/__drogular/developer-tools/components
/__drogular/assets/diagnostics.css
/__drogular/assets/diagnostics.js
```

### `stylesheet()`

Returns the built-in diagnostics stylesheet.

### `script()`

Returns the built-in browser client.

The script:

- fetches inspection JSON;
- fetches the browser renderer registry;
- renders built-in sections;
- dynamically imports registered custom renderer modules;
- falls back to JSON when a custom renderer is unavailable.

## Lifetime

All functions return `std::string_view` referring to static embedded resources. The views remain valid for the process lifetime.

## Related Types

- [`DiagnosticsPage`](diagnostics-page.md)
- [`DeveloperToolsComponentRegistry`](developer-tools-component-registry.md)
- [`ApplicationInspection`](application-inspection.md)
