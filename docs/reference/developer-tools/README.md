# Developer Tools

Developer Tools expose a structured inspection contract for a running Drogular application and provide a built-in diagnostics UI for that contract.

The subsystem is intentionally split into two layers:

```text
Application
    │
    ▼
ApplicationInspection
    │
    ├── JSON contract ─────► IDE / CLI / external tools
    │
    └── DiagnosticsPage ───► Browser UI
              │
              ▼
DeveloperToolsComponentRegistry
```

The inspection data is independent from the browser UI. Custom contributors add semantic sections to the inspection contract, while browser-side renderer modules are registered separately.

## Core Types

- [`ApplicationInspection`](application-inspection.md) — snapshot of routes, components, services, diagnostics, and extension sections.
- [`DeveloperToolsContributor`](developer-tools-contributor.md) — extension point for application-specific inspection data.
- [`DeveloperToolsContributors`](developer-tools-contributor.md#developertoolscontributors) — ordered collection of inspection contributors.
- [`DeveloperToolsComponentRegistry`](developer-tools-component-registry.md) — registry of trusted browser-side renderers for custom sections.
- [`DiagnosticsPage`](diagnostics-page.md) — built-in browser client at `/__drogular`.
- [`ApplicationInspectionController`](application-inspection-controller.md) — public JSON endpoint controller at `/__drogular/inspection`.
- [`diagnostics_resources`](diagnostics-resources.md) — embedded HTML, stylesheet, and JavaScript used by the diagnostics application.
- [`Diagnostics`](diagnostics.md) — shared diagnostic collection primitives surfaced by inspection.

## Activation

The complete subsystem can be enabled explicitly:

```cpp
drogular::App app;
app.enableDeveloperTools();
```

Selecting the Development profile also enables Developer Tools automatically unless they were disabled first:

```cpp
app.profile(drogular::ApplicationProfile::Development);
```

`enableInspection()` can be used when only the machine-readable inspection contract is required, without the browser diagnostics page.

## Extension Flow

A custom section is normally added in two independent steps:

```cpp
app.developerToolsContributor(
    std::make_shared<SystemInfoContributor>()
);

app.developerToolsComponent(
    "example.system-info",
    "/assets/system-info.js"
);
```

The contributor publishes data and names the semantic renderer. The component registry maps that renderer name to a trusted browser module.

This separation lets external tools consume the same JSON without depending on the diagnostics UI.

## Built-in Endpoints

| Endpoint | Purpose |
|---|---|
| `/__drogular` | Diagnostics browser application |
| `/__drogular/inspection` | Application inspection JSON |
| `/__drogular/developer-tools/components` | Browser renderer registry JSON |
| `/__drogular/assets/diagnostics.css` | Built-in stylesheet |
| `/__drogular/assets/diagnostics.js` | Built-in browser client |

## See Also

- [`App`](../application/app.md)
- [`ApplicationProfile`](../application/application-profile.md)
- [`ApplicationServices`](../dependency-injection/application-services.md)
- [`Developer Tools example`](../../../examples/developer_tools/README.md)
