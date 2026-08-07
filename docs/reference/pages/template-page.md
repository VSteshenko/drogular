# `TemplatePage`

**Namespace:** `drogular`  
**Header:** `<drogular/page.hpp>`  
**Kind:** Class

## Purpose

`TemplatePage` is the base class for pages rendered through Drogular's template pipeline.

It combines the route-facing `Page` API with `TemplateRenderable`, which provides inline templates, external template files, optional layouts, preprocessing, compilation, caching, and nested component expansion.

---

## Synopsis

```cpp
class TemplatePage : public Page, public TemplateRenderable {
public:
    std::string render(RenderContext& context) override;
};
```

`render()` delegates to the shared template pipeline.

---

## Template Sources

Override one of the following methods inherited from `TemplateRenderable`.

### `templateHtml()`

```cpp
virtual std::string templateHtml() const;
```

Returns inline template source when `templatePath()` is empty.

```cpp
std::string templateHtml() const override {
    return "<h1>{{ pageTitle }}</h1>";
}
```

### `templatePath()`

```cpp
virtual std::string templatePath() const;
```

Returns an external template path.

```cpp
std::string templatePath() const override {
    return "todo.html";
}
```

A non-empty `templatePath()` takes precedence over `templateHtml()`.

Relative paths are resolved through the application's template root.

### `layoutPath()`

```cpp
virtual std::string layoutPath() const;
```

Returns an optional external layout path.

```cpp
std::string layoutPath() const override {
    return "layouts/main.html";
}
```

The default empty path renders the page without a layout.

---

## Rendering

### `render()`

```cpp
std::string render(RenderContext& context) override;
```

Runs the shared template pipeline and returns the final HTML.

The pipeline handles:

- source loading;
- optional layout composition;
- template preprocessing and includes;
- AST compilation and caching;
- context value rendering;
- nested component expansion.

Applications normally do not override `render()` on `TemplatePage`. Override `onInit()` and the template source methods instead.

---

## Example

TodoPWA defines a template-backed page with an external template and layout:

```cpp
class TodoPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override;

    std::string templatePath() const override {
        return "todo.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};
```

The page prepares values in `onInit()` and the template consumes them during `render()`.

```cpp
void TodoPage::onInit(drogular::RenderContext& context) {
    context.set("pageTitle", std::string("Todos"));
}
```

Register the page through `App`:

```cpp
app.page<TodoPage>("/");
```

---

## Errors and Caching

Template loading, preprocessing, compilation, or rendering failures may propagate through the template pipeline as framework errors.

External template caching follows `ApplicationOptions::templateCacheEnabled()` and the configured template root.

Each `TemplateRenderable` instance owns its template cache object. Because registered pages are reused between requests, cached template state persists with the page instance.

---

## Lifetime and Thread Safety

`TemplatePage` has the same shared-instance lifetime as `Page`.

Do not place request-specific values in page members. Use `RenderContext` instead.

Template rendering and cache behavior must also be considered when the same page is entered concurrently.

---

## Related Types

- [`Page`](page.md)
- [`PageSupport`](page-support.md)
- `TemplateRenderable` *(coming soon)*
- [`RenderContext`](../rendering/render-context.md)
- `TemplateComponent` *(coming soon)*
