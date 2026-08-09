# `Router`

**Namespace:** `drogular`  
**Header:** `<drogular/router.hpp>`  
**Kind:** Class

## Purpose

`Router` connects Drogular pages, actions, static-file mappings, and the service worker to Drogon's HTTP routing layer.

It also records route metadata used by application inspection and Developer Tools.

---

## Role in Drogular

`Router` is the bridge between Drogon's transport-level routing and Drogular's application abstractions.

```text
                 Drogon
                   │
                   ▼
                 Router
                   │
        ┌──────────┼──────────┬──────────────┐
        │          │          │              │
        ▼          ▼          ▼              ▼
       Page   ActionHandler  Static Files  Service Worker
        │          │
        ▼          ▼
RenderContext  ActionContext
```

[`App`](../application/app.md) owns a router internally and exposes the normal registration surface through `App::page()`, `App::action()`, static-file configuration, and PWA configuration.

Application code therefore rarely needs to construct or operate a `Router` directly.

---

## Synopsis

```cpp
class Router {
public:
    Router();
    explicit Router(ApplicationServices* services);

    using PageFactory = std::function<std::shared_ptr<Page>()>;
    using ActionFactory = std::function<std::shared_ptr<ActionHandler>()>;

    void page(
        const std::string& path,
        PageFactory factory,
        std::string target
    );

    void action(
        const std::string& path,
        ActionFactory factory,
        std::string target
    );

    void staticFiles(
        const std::string& routePrefix,
        const std::filesystem::path& directory
    );

    void serviceWorker(
        const std::filesystem::path& path
    );

    const std::vector<RouteInspection>& routes() const;
};
```

---

## Construction

### Default constructor

```cpp
Router();
```

Creates a router without an application service container.

Page and action contexts created by such a router receive no `ApplicationServices` pointer.

### Services-aware constructor

```cpp
explicit Router(
    ApplicationServices* services
);
```

Stores the supplied application service container and attaches it to page and action request contexts.

`App` constructs its internal router this way.

---

## Page Routes

### `page()`

```cpp
void page(
    const std::string& path,
    PageFactory factory,
    std::string target
);
```

Registers a page factory for `path`. `target` is recorded as route-inspection metadata.

A **fresh page instance is created for every matching request**.

For each request, the route callback:

1. creates a new [`RenderContext`](../rendering/render-context.md);
2. attaches `ApplicationServices` and the current Drogon request;
3. extracts named route parameters through [`RoutePattern`](route-pattern.md);
4. stores those parameters in the context;
5. creates a page by invoking the registered factory;
6. renders it through the shared component lifecycle pipeline;
7. returns the resulting string as `text/html`.

```cpp
router.page(
    "/users/{id}",
    [] {
        return std::make_shared<UserPage>();
    },
    "UserPage"
);
```

### Page lifecycle behavior

Pages use the same lifecycle runner as component trees and rendering tests:

```text
onInit() -> render() -> onDestroy()
```

`onDestroy()` is guaranteed after `onInit()` succeeds, including when rendering or child rendering throws. The original exception is then propagated.

Because each request receives a new page object, page members are request-local to that page instance. `RenderContext` remains the preferred place for values that need to flow through the rendering tree.

### HTTP method metadata

The route inspection entry for a page records method `GET`.

The current implementation calls Drogon's `registerHandler(path, callback)` without supplying an explicit method constraint. This is an implementation detail worth considering when changing routing behavior.

---

## Action Routes

### `action()`

```cpp
void action(
    const std::string& path,
    ActionFactory factory,
    std::string target
);
```

Registers an [`ActionHandler`](../actions/action-handler.md) factory for POST requests. `target` is recorded as route-inspection metadata.

For each matching request, the router:

1. creates an [`ActionContext`](../actions/action-context.md);
2. extracts route parameters through `RoutePattern`;
3. stores those parameters in the context;
4. creates a fresh action by invoking the registered factory;
5. calls `ActionHandler::handle()`;
6. converts the returned [`ActionResult`](../actions/action-result.md) through [`toHttpResponse()`](../actions/action-response.md);
7. translates thrown exceptions through the action error-response contract.

[`ActionValidationError`](../forms-and-validation/action-validation-error.md) becomes `400 Bad Request`; other exceptions become a safe `500 Internal Server Error`.

```cpp
router.action(
    "/users/{id}/delete",
    [] {
        return std::make_shared<DeleteUserAction>();
    },
    "DeleteUserAction"
);
```

Each request receives a new action object, so action members are not shared between concurrent requests.

---

## Route Parameters

Both page and action registration construct a `RoutePattern` from the registered path.

For a route such as:

```text
/projects/{projectId}/tasks/{taskId}
```

and request path:

```text
/projects/10/tasks/25
```

the router places these values into the corresponding request context:

```text
projectId = 10
taskId    = 25
```

Pages read them through `RenderContext::routeParam()` and actions through `ActionContext::routeParam()`.

`RoutePattern::match()` returns a success flag, but the current router callbacks do not inspect that return value before continuing. Normally Drogon's registered route has already selected the handler.

---

## Static Files

### `staticFiles()`

```cpp
void staticFiles(
    const std::string& routePrefix,
    const std::filesystem::path& directory
);
```

Registers a GET handler rooted at `directory` under the supplied route prefix.

```cpp
router.staticFiles(
    "/assets",
    "public"
);
```

A trailing `/` is removed from the route prefix before the handler path is registered.

The implementation:

- canonicalizes the configured root directory;
- rejects paths that escape that root with HTTP 403;
- returns HTTP 404 for missing or non-regular files;
- resolves the file through `StaticFileResolver`;
- applies current static-file cache, ETag, and Last-Modified options when an `ApplicationOptions` instance is available;
- supports conditional 304 responses for matching ETag or Last-Modified request headers.

Static-file behavior is documented in detail in the [Static Files](../static-files/README.md) reference.

---

## Service Worker

### `serviceWorker()`

```cpp
void serviceWorker(
    const std::filesystem::path& path
);
```

Registers the supplied file at the fixed application route:

```text
/service-worker.js
```

The response uses content type `application/javascript` and adds:

```text
Service-Worker-Allowed: /
```

The source file may live anywhere on disk; its public route remains `/service-worker.js`.

---

## Route Inspection

### `routes()`

```cpp
const std::vector<RouteInspection>& routes() const;
```

Returns the router's accumulated inspection records.

A record is appended when a page, action, static-file mapping, or service-worker route is registered.

The returned reference aliases the router's internal vector.

`RouteInspection` and `RouteKind` belong to the Developer Tools inspection API and will be documented in that section.

---

## Lifetime and Thread Safety

`Router` is intended to be configured during application startup.

It contains no internal synchronization around the `routes_` vector. Do not modify route registrations concurrently with inspection reads or other route registration operations.

Registered page and action factories are captured by Drogon handlers. Each matching request creates a fresh page or action instance, so mutable members of those instances are request-local. Objects resolved from shared services may still be shared and must follow their own thread-safety contracts.

---

## Example

Application code normally registers routes through `App`:

```cpp
int main() {
    drogular::App app;

    app.page<HomePage>("/")
       .page<UserPage>("/users/{id}")
       .action<CreateUserAction>("/users/create")
       .action<DeleteUserAction>("/users/{id}/delete");

    app.run(8080);
}
```

Conceptually, those calls delegate page and action registration to the application's internal `Router`.

---

## Related Types

- [`App`](../application/app.md)
- [`RoutePattern`](route-pattern.md)
- [`Page`](../pages/page.md)
- [`ActionHandler`](../actions/action-handler.md)
- [`RenderContext`](../rendering/render-context.md)
- [`ActionContext`](../actions/action-context.md)
- `RouteInspection` *(Developer Tools reference coming soon)*

## See Also

- [Routing — Getting Started](../../getting-started/routing.md)
