# `TemplateRenderable`

**Namespace:** `drogular`  
**Header:** `<drogular/template_renderable.hpp>`  
**Kind:** Base class

## Purpose

`TemplateRenderable` provides the shared template-rendering contract used by `TemplatePage` and `TemplateComponent`.

It owns the common pipeline for selecting template source, composing an optional layout, preprocessing includes, compiling and caching the template, evaluating it against `RenderContext`, and expanding nested components.

## Role in Drogular

```text
TemplatePage / TemplateComponent
              │
              ▼
      TemplateRenderable
              │
              ├── template source
              ├── optional layout
              ├── preprocessing
              ├── compilation/cache
              ├── RenderContext evaluation
              └── component expansion
```

Applications normally use this behavior through `TemplatePage` or `TemplateComponent` rather than deriving from `TemplateRenderable` directly.

## Public API

### `templateHtml()`

```cpp
virtual std::string templateHtml() const;
```

Returns inline template source. The default implementation returns an empty string.

It is used when `templatePath()` is empty.

### `templatePath()`

```cpp
virtual std::string templatePath() const;
```

Returns the path of an external template file. The default implementation returns an empty string.

A non-empty path takes precedence over `templateHtml()`.

### `layoutPath()`

```cpp
virtual std::string layoutPath() const;
```

Returns the path of an optional layout template. The default implementation returns an empty string.

## Protected API

### `renderTemplate()`

```cpp
std::string renderTemplate(RenderContext& context) const;
```

Executes the complete template pipeline and returns the rendered HTML.

`TemplatePage::render()` and `TemplateComponent::render()` delegate to this method.

## Caching and Lifetime

Each `TemplateRenderable` instance owns its own compiled-template cache.

For route-mounted `TemplatePage` objects, the instance is request-scoped, so this compiler cache is also scoped to that page instance. Shared external template-source caching remains controlled by `ApplicationOptions` and application services.

The lifetime of a `TemplateComponent` depends on how the component is created and rendered.

## Thread Safety

`TemplateRenderable` does not provide internal synchronization around its per-instance compiled-template cache. Do not share one mutable instance concurrently unless the surrounding ownership model guarantees safe access.

## Related Types

- [`TemplatePage`](../pages/template-page.md)
- [`TemplateComponent`](../components/template-component.md)
- [`RenderContext`](render-context.md)
- [`CompiledTemplate`](compiled-template.md)
