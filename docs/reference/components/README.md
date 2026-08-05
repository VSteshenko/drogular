# Components

Components render reusable pieces of HTML inside pages and other components.

The Components API provides a base rendering contract, a template-backed implementation, a static HTML component, and a registry that maps template tags to component factories.

---

## Types

- [`Component`](component.md) — base class for reusable UI components.
- [`TemplateComponent`](template-component.md) — component base class backed by Drogular's template pipeline.
- [`HtmlComponent`](html-component.md) — immutable component that returns a fixed HTML string.
- [`ComponentRegistry`](component-registry.md) — maps component tag names to factories.

---

## Typical Flow

```text
Page or template
      │
      ▼
<ComponentTag input="value" />
      │
      ▼
ComponentRegistry
      │
      ├── creates a new component instance
      ├── applies string inputs
      └── runs component lifecycle
              │
              ▼
          rendered HTML
```

Registered template tags create a fresh component instance for every occurrence. The component receives a child `RenderContext`, runs `onInit()`, renders, expands child slots, and then runs `onDestroy()`.

---

## Registration

Register a component through `App`:

```cpp
app.component<TodoItemComponent>();
app.component<AlertComponent>("Alert");
```

The no-argument overload uses `ComponentType::tag`.

A registered component type must inherit from `drogular::Component` and be default-constructible.

---

## Getting Started

- [Components](../../getting-started/components.md)
- [Project Structure](../../getting-started/project-structure.md)

## Related Reference

- [App](../application/app.md)
- [Pages](../pages/README.md)
- Rendering *(coming soon)*
- [State Management](../state-management/README.md)
- Testing *(coming soon)*
