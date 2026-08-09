# `renderComponentTree()`

**Namespace:** `drogular::test`  
**Header:** `<drogular/testing.hpp>`  
**Kind:** Function

## Purpose

`renderComponentTree()` runs a component lifecycle and renders its C++ child hierarchy into default or named slots.

It is used directly in component tests and is also used internally by Drogular's component rendering pipeline.

## Synopsis

```cpp
std::string renderComponentTree(
    Component& component,
    RenderContext& context
);
```

## Parameters

### `component`

The component whose lifecycle and child tree should be rendered.

### `context`

The render context used for the root component.

Each child returned by `Component::children()` is rendered with `context.createChild()`.

## Lifecycle

For every component processed by the helper:

```text
onInit(context)
      │
      ▼
render(context)
      │
      ▼
render child components
      │
      ▼
resolve slots
      │
      ▼
onDestroy(context)
```

Unlike current production page routing, this helper explicitly invokes `onDestroy()`.

## Default Children

Default child HTML comes from two sources:

1. the component parameter `__slot`, when present;
2. children whose `slot()` value is empty.

When the rendered HTML contains:

```html
<slot/>
```

the **first** occurrence is replaced by the combined default child HTML.

If no default slot exists, the default child HTML is appended to the end of the component HTML.

## Named Slots

A child with a non-empty `slot()` value contributes to the matching named slot:

```html
<slot name="header"/>
```

All occurrences of a named slot placeholder are replaced.

Multiple children targeting the same slot are concatenated in child order.

Named child HTML is not appended automatically when the corresponding named slot is absent.

## Child Contexts

Each child is rendered through:

```cpp
auto childContext = context.createChild();
```

This preserves parent-value lookup while isolating child-local values and scoped-service caches according to `RenderContext` child-context semantics.

## Behavior and Limitations

`renderComponentTree()` renders the explicit `Component::children()` hierarchy and slot placeholders.

It does not itself perform route handling or create HTTP responses.

Custom component tags embedded in arbitrary HTML are expanded by the component/template rendering pipeline, not by this helper alone.

Exceptions from component lifecycle methods, child rendering, or context operations propagate to the caller.

## Example

```cpp
class Layout final : public drogular::Component {
public:
    std::string render(
        drogular::RenderContext&
    ) override {
        return "<main><slot/></main>";
    }

    std::vector<std::shared_ptr<drogular::Component>>
    children() override {
        return {
            std::make_shared<Content>()
        };
    }

private:
    class Content final : public drogular::Component {
    public:
        std::string render(
            drogular::RenderContext&
        ) override {
            return "<h1>Hello</h1>";
        }
    };
};

drogular::RenderContext context;
Layout layout;

const auto html =
    drogular::test::renderComponentTree(
        layout,
        context
    );

EXPECT_EQ(
    html,
    "<main><h1>Hello</h1></main>"
);
```

## Related Types

- [`Component`](../components/component.md)
- [`RenderContext`](../rendering/render-context.md)
- [`renderPage()`](render-page.md)
