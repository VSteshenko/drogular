# `TemplateComponent`

**Namespace:** `drogular`  
**Header:** `<drogular/component.hpp>`  
**Kind:** Abstract convenience class

## Purpose

`TemplateComponent` renders a component through Drogular's shared template pipeline.

It combines the component lifecycle and input model from `Component` with template source loading, optional layouts, preprocessing, compilation and caching, template evaluation, and nested component expansion.

---

## Synopsis

```cpp
class TemplateComponent
    : public Component,
      public TemplateRenderable {
public:
    std::string render(RenderContext& context) override;
};
```

A concrete template component normally overrides `templatePath()` or `templateHtml()` inherited from `TemplateRenderable`.

---

## Rendering Behavior

`render()` performs two steps:

```cpp
std::string TemplateComponent::render(RenderContext& context) {
    applyParams(context);
    return renderTemplate(context);
}
```

1. Every stored parameter or input is copied into the supplied `RenderContext`.
2. The shared template pipeline renders the selected template.

Because parameters are applied first, an input can be referenced directly by its key in the template.

```cpp
component.setInput("title", std::string("Users"));
```

```html
<h1>{{ title }}</h1>
```

---

## Template Source

### Inline template

```cpp
std::string templateHtml() const override {
    return "<strong>{{ label }}</strong>";
}
```

### External template

```cpp
std::string templatePath() const override {
    return "components/todo_item.html";
}
```

When `templatePath()` is non-empty, it takes precedence over `templateHtml()`.

The path is resolved through the application's configured template root when application services and options are attached to the context.

### Layout

```cpp
std::string layoutPath() const override {
    return "layouts/card.html";
}
```

An optional layout replaces its first `@content` marker with the component template source before preprocessing and compilation.

---

## Template Pipeline

The current pipeline is:

```text
templatePath() or templateHtml()
          │
          ▼
optional layout composition
          │
          ▼
TemplatePreprocessor
          │
          ▼
compiled template cache
          │
          ▼
RenderContext evaluation
          │
          ▼
nested component expansion
```

Each `TemplateRenderable` instance owns its compiled-template cache. External template source caching is controlled by `ApplicationOptions` and the application's services.

---

## Inputs

Declarative component attributes are evaluated in the parent context and supplied as string inputs:

```html
<PortalSelect
    options="{{ userRoleOptionsPath }}"
    id="role"
    required="true"
/>
```

Read them during `onInit()`:

```cpp
void onInit(drogular::RenderContext& context) override {
    const auto selectId =
        input<std::string>("id").value_or("");

    context.set("selectId", selectId);
}
```

For programmatically created components, inputs may use any copyable type accepted by `std::any`.

---

## Example

```cpp
class TodoItemComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "TodoItem";

    std::string templatePath() const override {
        return "components/todo_item.html";
    }

    void onInit(drogular::RenderContext& context) override {
        const auto done =
            input<std::string>("done").value_or("false");

        context.set(
            "marker",
            done == "true" ? std::string("[x]") : std::string("[ ]")
        );
    }
};
```

---

## Errors

Template loading, preprocessing, compilation, and evaluation errors follow the shared template pipeline behavior. They are not converted into component-specific result objects by `TemplateComponent::render()`.

---

## Related Types

- [`Component`](component.md)
- [`ComponentRegistry`](component-registry.md)
- [`TemplateRenderable`](../rendering/template-renderable.md)
- [`RenderContext`](../rendering/render-context.md)
- [`TemplatePage`](../pages/template-page.md)
