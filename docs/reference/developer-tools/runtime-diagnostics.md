# `RuntimeDiagnostics`

**Namespace:** `drogular`  
**Header:** `<drogular/runtime_diagnostics.hpp>`  
**Kind:** Thread-safe aggregate runtime collector

## Purpose

`RuntimeDiagnostics` records inexpensive application-wide counters for the Drogular request pipeline. It deliberately 
does not depend on HTTP objects, JSON, `ApplicationInspection`, or the Developer Tools UI. Runtime code records events;
observability code consumes immutable snapshots.

The collector stores aggregates only. It does not retain request history, request bodies, route values, user data, or 
per-request traces.

## Snapshot

```cpp
struct RuntimeDiagnosticsSnapshot {
    RequestRuntimeDiagnostics requests;
    InteractionRuntimeDiagnostics interactions;
    RenderingRuntimeDiagnostics rendering;
    ServiceScopeRuntimeDiagnostics serviceScopes;
};

RuntimeDiagnosticsSnapshot snapshot() const noexcept;
```

`snapshot()` is safe to call while requests are being processed. Counters are maintained atomically.

## Request counters

`RequestRuntimeDiagnostics` contains:

- `requests` — matched Drogular page/action executions; static-file and service-worker handling are not included;
- `actions` — the subset dispatched to an `ActionHandler`;
- `renderedActions` — actions whose `ActionRenderer` component render completed successfully;
- `redirects` — action responses classified as redirects.

## Interaction counters

`InteractionRuntimeDiagnostics` contains:

- `requests` — action requests marked as Drogular Interactions;
- `getRequests` and `postRequests` — Interaction requests by supported method;
- `successfulResponses` — Interaction responses in the 2xx class;
- `clientErrorResponses` — Interaction responses in the 4xx class;
- `redirects` — Interaction responses in the 3xx class.

Redirects are kept separate from successful responses. Server errors are not folded into `clientErrorResponses`.

## Rendering counters

`RenderingRuntimeDiagnostics` contains:

- `componentsRendered` — component nodes whose render lifecycle is entered by the component-tree renderer;
- `renderFailures` — failed top-level component-tree render operations;
- `maximumDepth` — greatest component-tree depth observed by the collector. Root depth is `1`.

A failed `ActionRenderer` operation increments the rendering failure aggregate but does not increment `renderedActions`.

## Scoped-service counters

`ServiceScopeRuntimeDiagnostics` contains:

- `resolutions` — scoped-service resolution attempts;
- `cacheHits` — resolutions satisfied from the request scope cache;
- `servicesCreated` — scoped service instances created and inserted into that cache.

Only `ServiceScope` activity connected to a `RuntimeDiagnostics` collector contributes to these values. Standalone scopes 
may intentionally be uninstrumented.

## Application integration

A normal `App` owns the collector for its lifetime and wires it through `Router` into the shared request state and 
`ServiceScope`. Application code normally does not need to call the `record*()` functions itself.

`App::inspect()` exposes a snapshot through the built-in [`RuntimeDiagnosticsContributor`](runtime-diagnostics-contributor.md).

## Thread safety

Recording and snapshot operations are thread-safe. The collector is non-copyable and non-movable so references used 
by request infrastructure remain stable for its lifetime.

## Related Types

- [`ApplicationInspection`](application-inspection.md)
- [`RuntimeDiagnosticsContributor`](runtime-diagnostics-contributor.md)
- [`DiagnosticsPage`](diagnostics-page.md)
- [`ActionRenderer`](../actions/action-renderer.md)
