# `renderPage()`

**Namespace:** `drogular::test`  
**Header:** `<drogular/testing.hpp>`  
**Kind:** Function template

## Purpose

`renderPage<PageType>()` renders a default-constructible page directly in a test without registering a route or starting the Drogon server.

It can optionally attach `ApplicationServices` and a Drogon request to the generated `RenderContext`.

## Synopsis

```cpp
template <typename PageType>
RenderResult renderPage(
    ApplicationServices* services = nullptr,
    const drogon::HttpRequestPtr& request = nullptr
);
```

## Template Parameters

### `PageType`

The page type to construct and render.

`PageType` must be default-constructible and provide the normal page/component rendering interface.

## Parameters

### `services`

```cpp
ApplicationServices* services = nullptr;
```

Optional application service container.

When supplied, it is attached to the test `RenderContext` through `RenderContext::setServices()`.

This allows the page and template/component rendering pipeline to resolve registered services and components.

### `request`

```cpp
const drogon::HttpRequestPtr& request = nullptr;
```

Optional Drogon request.

When supplied, it is attached through `RenderContext::setRequest()` and can be used by page logic that reads request parameters, cookies, or other request metadata.

## Return Value

Returns a [`RenderResult`](render-result.md) containing:

- the rendered HTML;
- the final `RenderContext`.

## Behavior

The current implementation performs these steps:

```text
Default-construct PageType
        │
        ▼
Create RenderContext
        │
        ├── attach services (optional)
        └── attach request  (optional)
        │
        ▼
page.onInit(context)
        │
        ▼
renderComponentTree(page, context)
        │
        ├── page.onInit(context) again
        ├── page.render(context)
        ├── render children / slots
        └── page.onDestroy(context)
        │
        ▼
RenderResult
```

### Current double `onInit()` behavior

`renderPage()` calls `page.onInit(context)` directly and then calls [`renderComponentTree()`](render-component-tree.md), which invokes `onInit()` again.

Therefore page initialization currently runs **twice** in this helper.

This differs from production routing, where `Router` currently invokes page `onInit()` once and renders the page directly.

### Not a router integration test

`renderPage()` does not:

- register or match a route;
- populate route parameters;
- start Drogon;
- create an HTTP response;
- execute action handlers;
- exercise middleware or external networking.

Use it for rendering-focused tests rather than full HTTP integration tests.

## Example

TodoPWA configures the same application services used by its page and then renders it directly:

```cpp
drogular::ApplicationServices services;
drogular::ApplicationOptions options;

options.setTemplateRoot(
    std::filesystem::path(DROGULAR_SOURCE_DIR)
    / "examples/todo_pwa/templates"
);

services.setOptions(&options);
services.registerService<TodoStore>(
    std::make_shared<TodoStore>()
);
services.components()
    .registerComponent<TodoItemComponent>();
services.components()
    .registerComponent<TodoPaginationComponent>();

const auto result =
    drogular::test::renderPage<TodoPage>(
        &services
    );

EXPECT_TRUE(
    drogular::test::contains(
        result.html,
        "Create Drogular project skeleton"
    )
);
```

A request can be supplied when page behavior depends on request parameters:

```cpp
auto request =
    drogon::HttpRequest::newHttpRequest();

request->setParameter("page", "2");

const auto result =
    drogular::test::renderPage<TodoPage>(
        &services,
        request
    );
```

## Related Types

- [`RenderResult`](render-result.md)
- [`renderComponentTree()`](render-component-tree.md)
- [`Page`](../pages/page.md)
- [`RenderContext`](../rendering/render-context.md)
- [`ApplicationServices`](../dependency-injection/application-services.md)
