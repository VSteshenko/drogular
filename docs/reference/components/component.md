# `Component`

**Namespace:** `drogular`  
**Header:** `<drogular/component.hpp>`  
**Kind:** Abstract class

## Purpose

`Component` is the base class for reusable UI elements.

A component renders HTML from a `RenderContext`, may expose lifecycle hooks, accept typed parameters, and provide child components for default or named slots.

---

## Role in Drogular

`Component` is the reusable unit of Drogular's server-side UI composition model.

Templates and component trees create component instances, supply inputs, render child components and slots, and combine the resulting HTML into the final page output.

```text
Template / Component Tree
          │
          ▼
 ComponentRegistry
          │
          ▼
      Component
          │
          ├── Inputs / Params
          ├── Lifecycle
          ├── Children
          └── Slots
          │
          ▼
     Rendered HTML
```

Components operate within a `RenderContext`, which provides request-scoped values and access to framework services used during rendering.

---

## Synopsis

```cpp
class Component {
public:
    virtual ~Component() = default;

    virtual void onInit(RenderContext& context);
    virtual void onDestroy(RenderContext& context);
    virtual std::string render(RenderContext& context) = 0;

    virtual std::vector<std::shared_ptr<Component>> children();
    virtual std::string slot() const;

    template <typename T>
    void setParam(std::string key, T value);

    template <typename T>
    std::optional<T> param(const std::string& key) const;

    void applyParams(RenderContext& context) const;

    template <typename T>
    void setInput(std::string key, T value);

    template <typename T>
    std::optional<T> input(const std::string& key) const;
};
```

---

## Lifecycle

When a component is rendered through Drogular's component-tree renderer, the lifecycle order is:

```text
onInit()
   │
   ▼
render()
   │
   ├── render child components
   └── fill default and named slots
   │
   ▼
onDestroy()
```

### `onInit()`

```cpp
virtual void onInit(RenderContext& context);
```

Called before `render()`. The default implementation does nothing.

Use it to resolve inputs and prepare values in the component's render context:

```cpp
void onInit(drogular::RenderContext& context) override {
    const auto done = input<std::string>("done").value_or("false");

    context.set(
        "marker",
        done == "true" ? std::string("[x]") : std::string("[ ]")
    );
}
```

### `render()`

```cpp
virtual std::string render(RenderContext& context) = 0;
```

Returns the component HTML.

Calling `render()` directly does not automatically run `onInit()`, render children, fill slots, or call `onDestroy()`. Use the normal template/component pipeline or `drogular::test::renderComponentTree()` when the complete lifecycle is required.

### `onDestroy()`

```cpp
virtual void onDestroy(RenderContext& context);
```

Called after the component and its child tree have been rendered. The default implementation does nothing.

For nested components, child destruction completes before parent destruction.

---

## Parameters and Inputs

### `setParam()`

```cpp
template <typename T>
void setParam(std::string key, T value);
```

Stores a typed value in the component's internal parameter map.

Values are stored using `std::any`.

```cpp
component.setParam("title", std::string("Users"));
```

### `param()`

```cpp
template <typename T>
std::optional<T> param(const std::string& key) const;
```

Returns a copy of the stored value when both the key and requested type match.

It returns `std::nullopt` when:

- the key is absent;
- the stored type differs from `T`.

```cpp
const auto title = component.param<std::string>("title");
```

### `setInput()` and `input()`

```cpp
template <typename T>
void setInput(std::string key, T value);

template <typename T>
std::optional<T> input(const std::string& key) const;
```

These are semantic aliases for `setParam()` and `param()`.

Template tag attributes are rendered first and then supplied as `std::string` inputs:

```html
<TodoItem done="{{ todoDone }}" />
```

```cpp
const auto done = input<std::string>("done");
```

### `applyParams()`

```cpp
void applyParams(RenderContext& context) const;
```

Copies every stored parameter into the supplied `RenderContext`.

Existing context values with the same keys are replaced. `TemplateComponent::render()` calls this automatically before rendering its template.

---

## Child Components and Slots

### `children()`

```cpp
virtual std::vector<std::shared_ptr<Component>> children();
```

Returns programmatically defined child components. The default implementation returns an empty vector.

Each child is rendered with a child `RenderContext`.

Children with an empty `slot()` value are inserted into `<slot/>`. If the parent HTML has no default slot marker, their HTML is appended to the parent output.

### `slot()`

```cpp
virtual std::string slot() const;
```

Returns the named slot assigned to this component when it is used as a programmatic child.

The default value is an empty string, which selects the default slot.

```cpp
std::string slot() const override {
    return "header";
}
```

The parent template may receive it through:

```html
<slot name="header"/>
```

Declarative non-self-closing component tags place their inner rendered HTML into the reserved `__slot` input and expose it through the default slot.

---

## Lifetime and Thread Safety

A component created from `ComponentRegistry` is a new instance for each component tag occurrence.

A component used directly may have any lifetime chosen by the application. Its parameter map is mutable and has no internal synchronization. Do not mutate or render the same instance concurrently without external synchronization.

`RenderContext` should contain request-specific render data. Avoid storing request-specific state in component members when an instance may be reused.

---

## Example

TodoPWA defines a template-backed item component:

```cpp
class TodoItemComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "TodoItem";

    std::string templatePath() const override {
        return "components/todo_item.html";
    }

    void onInit(drogular::RenderContext& context) override {
        const auto done =
            input<std::string>("done")
                .value_or("false");

        context.set(
            "marker",
            done == "true" ? std::string("[x]") : std::string("[ ]")
        );
    }
};
```

Register and use it:

```cpp
app.component<TodoItemComponent>();
```

```html
<TodoItem done="{{ todoDone }}" />
```

---

## Related Types

- [`TemplateComponent`](template-component.md)
- [`HtmlComponent`](html-component.md)
- [`ComponentRegistry`](component-registry.md)
- [`RenderContext`](../rendering/render-context.md)
- `TemplateRenderable` *(coming soon)*
- [`Testing`](../testing/README.md)
