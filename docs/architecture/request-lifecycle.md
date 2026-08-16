# Request Lifecycle

This document describes the ownership and lifetime model used by Drogular's primary HTTP paths.

## Page requests

`App::page<PageType>()` registers a factory with the router. The factory, not a Page instance, is retained by the route handler.

For every matching GET request the router:

```text
HTTP request
    │
    ▼
create RenderContext
    │
    ├── attach ApplicationServices
    ├── attach HttpRequest
    └── copy route parameters
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
create ActionContext
    │
    ├── attach ApplicationServices
    ├── retain HttpRequest
    └── copy route parameters
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

`RenderContext` is a rendering scope. It owns:

- template values;
- route parameters;
- request access;
- GraphQL render results;
- the scoped-service cache used by mutable `service<T>()`;
- a parent link for value lookup in child rendering contexts.

Child contexts inherit access to application services and GraphQL client configuration, but they maintain their own local values and scoped-service cache.

`ActionContext` is the command/request context. It owns request access, route parameters, form conversion helpers, session access, and service-container access. It does not currently implement the same scoped-service cache as `RenderContext`.

## Shared application services

`ApplicationServices` is application-lived. It stores component registration, service registrations/factories, the GraphQL client, application options, and shared template-source cache infrastructure.

This means thread-safety must be evaluated according to the registered service lifetime and the service implementation itself.

| Lifetime | Current ownership |
|---|---|
| `Singleton` | one application-shared instance |
| `LazySingleton` | one lazily-created application-shared instance |
| `Transient` | new instance for each `ApplicationServices::service<T>()` resolution |
| `Scoped` | factory in `ApplicationServices`; instance cached by the resolving mutable `RenderContext` |

The current `Scoped` lifetime is therefore not a universal request scope. In particular, `ActionContext::service<T>()` resolves through `ApplicationServices` and does not create scoped registrations, and child render contexts have independent scoped caches.

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
