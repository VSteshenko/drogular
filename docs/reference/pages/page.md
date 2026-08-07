# `Page`

**Namespace:** `drogular`  
**Header:** `<drogular/page.hpp>`  
**Kind:** Abstract class

## Purpose

`Page` is the base class for complete HTML pages mounted to GET routes.

It inherits the component lifecycle and rendering contract from `Component` and adds an optional GraphQL query declaration.

---

## Synopsis

```cpp
class Page : public Component {
public:
    ~Page() override = default;

    virtual std::optional<gql::Query> query() const;
};
```

A concrete page must implement `Component::render(RenderContext&)` directly or inherit from `TemplatePage`.

---

## Construction and Registration

Pages are normally registered through `App`:

```cpp
app.page<HomePage>("/");
```

`App::page<PageType>()` constructs the page with `std::make_shared<PageType>()`. The page type must therefore:

- inherit from `drogular::Page`;
- be default-constructible.

The created page object is retained by the router and reused for every matching request.

---

## Inherited Lifecycle

### `onInit()`

```cpp
virtual void onInit(RenderContext& context);
```

The router invokes `onInit()` before rendering each request.

Use it to resolve services and place request-specific values into the `RenderContext`:

```cpp
class HomePage final : public drogular::Page {
public:
    void onInit(drogular::RenderContext& context) override {
        context.set("title", std::string("Home"));
    }

    std::string render(drogular::RenderContext& context) override {
        return "<h1>" + context.get<std::string>("title").value_or("") + "</h1>";
    }
};
```

### `render()`

```cpp
virtual std::string render(RenderContext& context) = 0;
```

Returns the complete response body.

The router sends the returned string with content type `text/html`.

### `onDestroy()`

```cpp
virtual void onDestroy(RenderContext& context);
```

`Page` inherits this hook from `Component`, but the current HTTP page route implementation does **not** call it.

Do not rely on `onDestroy()` for request cleanup. Prefer automatic local lifetimes or explicit cleanup inside the request flow.

---

## Public API

### `query()`

```cpp
virtual std::optional<gql::Query> query() const;
```

Returns the GraphQL query associated with the page.

The default implementation returns `std::nullopt`.

```cpp
const auto query = page.query();
```

In the current router implementation, registering and rendering a page does not automatically execute the value returned by `query()`. Pages that need GraphQL data should use the current GraphQL client or provider APIs explicitly from their request flow.

---

## Request Context

Before `onInit()` runs, the router creates a `RenderContext` and attaches:

- the application's service container;
- the current Drogon request;
- route parameters extracted from the registered pattern.

```cpp
const auto request = context.request();
const auto id = context.routeParam("id");
```

Request data should remain in the context or in local variables.

---

## Lifetime and Thread Safety

A page is instantiated once when its route is registered, not once per request.

The same object may serve multiple requests and may be entered concurrently by Drogon's request-processing threads. Avoid storing request-specific or mutable render state in page members.

Prefer:

```cpp
void onInit(drogular::RenderContext& context) override {
    context.set("pageTitle", std::string("Users"));
}
```

Avoid:

```cpp
std::string currentUser_; // Shared across requests.
```

Any mutable page-level state requires external synchronization.

---

## Example

The Developer Tools example renders HTML directly:

```cpp
class DeveloperToolsExamplePage final : public drogular::Page {
public:
    std::string render(drogular::RenderContext&) override {
        return R"HTML(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>Drogular Developer Tools Example</title>
</head>
<body>
  <h1>Drogular Developer Tools Example</h1>
  <p><a href="/__drogular">Open Developer Tools</a></p>
</body>
</html>)HTML";
    }
};
```

Register it as a GET route:

```cpp
app.page<DeveloperToolsExamplePage>("/");
```

---

## Related Types

- [`TemplatePage`](template-page.md)
- [`PageSupport`](page-support.md)
- [`Component`](../components/component.md)
- [`RenderContext`](../rendering/render-context.md)
- [`App`](../application/app.md)
