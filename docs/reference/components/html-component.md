# `HtmlComponent`

**Namespace:** `drogular`  
**Header:** `<drogular/component.hpp>`  
**Kind:** Final class

## Purpose

`HtmlComponent` is a minimal component that returns a fixed HTML string.

It is useful for programmatic component trees, tests, and small immutable fragments that do not need template evaluation or context data.

---

## Synopsis

```cpp
class HtmlComponent final : public Component {
public:
    explicit HtmlComponent(std::string html);

    std::string render(RenderContext& context) override;
};
```

---

## Construction

```cpp
auto component = std::make_shared<drogular::HtmlComponent>(
    "<strong>Ready</strong>"
);
```

The HTML string is moved into the component and cannot be changed through the public API.

---

## `render()`

```cpp
std::string render(RenderContext& context) override;
```

Returns the stored HTML unchanged.

The supplied `RenderContext` is ignored. No escaping, template evaluation, preprocessing, or nested component-tag expansion is performed by this method itself.

When rendered as part of a component tree, the normal lifecycle and slot processing still apply around the returned string.

---

## Registration

`HtmlComponent` is not default-constructible, so it cannot be registered directly through `App::component<ComponentType>()` or `ComponentRegistry::registerComponent<ComponentType>()`, whose factories use `std::make_shared<ComponentType>()` with no arguments.

Use it programmatically instead, or create a default-constructible wrapper component for registry-based use.

---

## Example

```cpp
class LayoutComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<main><slot/></main>";
    }

    std::vector<std::shared_ptr<drogular::Component>> children() override {
        return {
            std::make_shared<drogular::HtmlComponent>(
                "<p>Hello from a child component.</p>"
            )
        };
    }
};
```

The rendered result is:

```html
<main><p>Hello from a child component.</p></main>
```

---

## Thread Safety

After construction, `HtmlComponent` does not mutate its HTML value. The inherited parameter map remains mutable, but `render()` does not read it.

Concurrent use of the same instance is safe only when inherited mutable APIs such as `setParam()` are not used concurrently.

---

## Related Types

- [`Component`](component.md)
- [`TemplateComponent`](template-component.md)
- Testing *(coming soon)*
