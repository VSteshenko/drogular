# Request Lifecycle

This document describes the ownership and lifetime model used by Drogular's primary HTTP paths.

## Page requests

`App::page<PageType>()` registers a factory with the router. The factory, not a Page instance, is retained by the route handler.

For every matching GET request the router:

```text
HTTP request
    │
    ▼
create request state
    │
    ├── attach ApplicationServices
    ├── retain HttpRequest
    ├── attach RuntimeDiagnostics
    ├── create request ServiceScope
    └── copy route parameters
    │
    ▼
create RenderContext over request state
    │
    ▼
create fresh Page
    │
    ▼
onInit(context)
    │
    ▼
render(context)
    │
    ├── template evaluation
    ├── nested components
    └── child RenderContexts
    │
    ▼
onDestroy(context)
    │
    ▼
HTML response
```

`onDestroy()` is guaranteed after successful `onInit()`, including when rendering throws. The rendering exception continues to propagate after cleanup.

A Page instance is therefore request-scoped. Application-shared objects referenced by that Page are not made thread-safe by this lifetime.

## Action requests

`App::action<ActionType>()` also registers a factory. For every matching POST request the router:

```text
HTTP request
    │
    ▼
create request state
    │
    ├── attach ApplicationServices
    ├── retain HttpRequest
    ├── attach RuntimeDiagnostics
    ├── create request ServiceScope
    └── copy route parameters
    │
    ▼
create ActionContext over request state
    │
    ▼
create fresh ActionHandler
    │
    ▼
handle(context)
    │
    ▼
ActionResult
    │
    ▼
toHttpResponse()
```

Expected validation failures use `ActionValidationError` and become `400 Bad Request`. Other exceptions are logged and converted to a safe `500 Internal Server Error`.

## Context ownership

`RenderContext` is a rendering scope. It owns local template values, GraphQL render results, and a parent link for value 
lookup. Request-bound state — the HTTP request, route parameters, application services, runtime diagnostics, and 
`ServiceScope` — is held in framework-owned shared request state.

Child contexts share that request state while maintaining their own local render values.

`ActionContext` is the command/request view over the same kind of request state and adds form conversion helpers, session access, and action-oriented service access. When `ActionRenderer` renders a component from an action, the bridge creates a `RenderContext` over the **same** request state, so scoped services and diagnostics remain request-consistent.

## Shared application services

`ApplicationServices` is application-lived. It stores component registration, service registrations/factories, the GraphQL client, application options, and shared template-source cache infrastructure.

This means thread-safety must be evaluated according to the registered service lifetime and the service implementation itself.

| Lifetime | Current ownership |
|---|---|
| `Singleton` | one application-shared instance |
| `LazySingleton` | one lazily-created application-shared instance |
| `Transient` | new instance for each `ApplicationServices::service<T>()` resolution |
| `Scoped` | factory in `ApplicationServices`; one instance per request-owned `ServiceScope` |

`RenderContext` and `ActionContext` both resolve scoped registrations through a request-owned scope. Child render contexts share their parent's scope. A scoped service can therefore be treated as one instance per HTTP request in the framework request pipeline.

## Runtime diagnostics

`App` owns one application-lived `RuntimeDiagnostics` collector. The router attaches it to each framework request state. 
Runtime subsystems update inexpensive aggregate counters rather than retaining request history.

The current aggregates cover routed page/action requests, rendered actions and redirects, component rendering and maximum
component depth, Drogular Interactions request/response classes, and scoped-service resolutions/cache hits/creations. 
`App::inspect()` publishes an immutable snapshot through Developer Tools.

Standalone `ActionContext`, `RenderContext`, and `ServiceScope` objects may be created without runtime diagnostics; 
this is supported for tests and low-level use and is not treated as an invariant violation.

## Sessions

`SessionStore` and individual `Session` objects synchronize their internal maps. They can be shared across concurrent requests.

Thread-safe session storage does not make values stored outside those objects thread-safe, and it does not define application identity policy. `AuthSupport` determines the generic session-based authenticated state; application-specific identity and authorization remain application responsibilities.

## Testing parity

`test::renderPage()` and component rendering use the same production lifecycle runner. Tests therefore observe the same `onInit -> render -> onDestroy` behavior as routed Page rendering instead of maintaining a second lifecycle implementation.

## Current concurrency constraints

The lifetime model above does not by itself make every application-wide object thread-safe.

`ApplicationServices` synchronizes its service registration/resolution stores. Concurrent first resolution of the same `LazySingleton` is serialized per service type, so only one shared instance is created and published. Factories for unrelated lazy singleton types do not share that initialization lock.

`TemplateSourceCache`, although application-wide, synchronizes access to its internal source map and loader. Cached reads use shared locking, while cache misses, `clear()` and `setLoader()` use exclusive locking. `load()` returns the source by value so a later cache clear cannot invalidate data already handed to a renderer.

Application startup remains the intended phase for registration and framework configuration. Application singleton services with their own mutable state remain responsible for their own synchronization.
