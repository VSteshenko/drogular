# `RuntimeDiagnosticsContributor`

**Namespace:** `drogular`  
**Header:** `<drogular/developer_tools/runtime_diagnostics_contributor.hpp>`  
**Kind:** Built-in inspection contributor

## Purpose

`RuntimeDiagnosticsContributor` adapts a [`RuntimeDiagnostics`](runtime-diagnostics.md) snapshot to the extensible 
`ApplicationInspection::sections` contract. It is read-only and keeps the runtime collector independent from Developer 
Tools and JSON.

## Public API

```cpp
class RuntimeDiagnosticsContributor final
    : public DeveloperToolsContributor {
public:
    explicit RuntimeDiagnosticsContributor(
        const RuntimeDiagnostics& diagnostics
    ) noexcept;

    void contribute(
        ApplicationInspection& inspection
    ) const override;
};
```

The referenced collector must outlive the contributor.

## Published sections

One contribution adds or replaces these sections:

| Section id | Title | Renderer | Data |
|---|---|---|---|
| `runtime` | Runtime | `drogular.runtime` | request/action and rendering aggregates |
| `interactions` | Interactions | `drogular.interactions` | Interaction request/response aggregates |
| `service-scopes` | Service Scopes | `drogular.service-scopes` | scoped-service aggregates |

The renderer names are built into `DiagnosticsPage`; they do not require registration through `DeveloperToolsComponentRegistry`.

## `App::inspect()` integration

`App` owns its runtime collector and invokes the built-in contributor before application-registered `DeveloperToolsContributor` 
objects. This means custom contributors see an inspection that already contains the runtime sections.

The sections use the existing extension contract and therefore do not require a new `ApplicationInspection::SchemaVersion`.

Application code normally does not register `RuntimeDiagnosticsContributor` manually. The type remains useful when constructing 
inspection snapshots directly in tests or tooling.

## Related Types

- [`RuntimeDiagnostics`](runtime-diagnostics.md)
- [`ApplicationInspection`](application-inspection.md)
- [`DeveloperToolsContributor`](developer-tools-contributor.md)
- [`DiagnosticsPage`](diagnostics-page.md)
