# `DiagnosticsPage`

**Namespace:** `drogular`  
**Header:** `<drogular/developer_tools/diagnostics_page.hpp>`  
**Kind:** Final `Page`

## Purpose

`DiagnosticsPage` is Drogular's built-in browser client for the public application inspection contract.

It deliberately consumes the inspection JSON endpoint from JavaScript instead of reading [`ApplicationInspection`](application-inspection.md) directly on the server.

## Paths

```cpp
static constexpr const char* Path =
    "/__drogular";

static constexpr const char* AssetsPath =
    "/__drogular/assets";
```

When enabled, the application also exposes:

- `/__drogular/inspection`;
- `/__drogular/developer-tools/components`;
- `/__drogular/assets/diagnostics.css`;
- `/__drogular/assets/diagnostics.js`.

## Rendering

```cpp
std::string render(
    RenderContext& context
) override;
```

The current implementation ignores `RenderContext` and returns the embedded HTML from [`diagnostics_resources::pageHtml()`](diagnostics-resources.md).

The HTML then loads the built-in browser client, which fetches inspection and renderer-registry JSON with `cache: 'no-store'`.

## Activation

Normally enabled through:

```cpp
app.enableDeveloperTools();
```

or:

```cpp
app.enableDiagnosticsPage();
```

`enableDiagnosticsPage()` automatically enables the inspection contract first.

The Development profile also enables the complete Developer Tools subsystem unless it was explicitly disabled beforehand.

## Extension Rendering

Built-in inspection sections use built-in renderer names such as:

```text
drogular.routes
drogular.components
drogular.services
drogular.diagnostics
```

Custom sections can request a renderer registered through [`DeveloperToolsComponentRegistry`](developer-tools-component-registry.md).

Failure to load a custom renderer does not prevent the diagnostics application from rendering other sections; that section falls back to JSON output.

## Behavior

`DiagnosticsPage` is a diagnostic client, not the inspection source of truth. External tools can consume `/__drogular/inspection` without loading this page.

The current built-in page has no authentication or authorization layer of its own. Applications exposing Developer Tools should treat these endpoints according to their deployment and network-security requirements.

## Related Types

- [`ApplicationInspection`](application-inspection.md)
- [`ApplicationInspectionController`](application-inspection-controller.md)
- [`DeveloperToolsComponentRegistry`](developer-tools-component-registry.md)
- [`diagnostics_resources`](diagnostics-resources.md)
- [`Page`](../pages/page.md)
