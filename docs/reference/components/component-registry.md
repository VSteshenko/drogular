# `ComponentRegistry`

**Namespace:** `drogular`  
**Header:** `<drogular/component_registry.hpp>`  
**Kind:** Class

## Purpose

`ComponentRegistry` maps template tag names to factories that create component instances.

The application owns a registry through `ApplicationServices`. Most applications register components through `App::component()`, while direct registry access is useful for infrastructure, tests, and inspection.

---

## Synopsis

```cpp
class ComponentRegistry {
public:
    template <typename ComponentType>
    void registerComponent(std::string tag);

    template <typename ComponentType>
    void registerComponent();

    std::shared_ptr<Component> create(const std::string& tag) const;
    bool contains(const std::string& tag) const;

    const Diagnostics& diagnostics() const;
    void clearDiagnostics();

    std::vector<std::string> tags() const;
};
```

---

## Registration

### Explicit tag

```cpp
registry.registerComponent<AlertComponent>("Alert");
```

### Static tag member

```cpp
class TodoItemComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "TodoItem";
};

registry.registerComponent<TodoItemComponent>();
```

The no-argument overload uses `ComponentType::tag`.

The registered type must:

- derive from `drogular::Component`;
- be default-constructible.

The factory creates a new instance with `std::make_shared<ComponentType>()` every time `create()` is called.

Registering an already used tag replaces the previous factory and records warning `DGL-CMP-001`.

---

## Creation and Lookup

### `create()`

```cpp
std::shared_ptr<Component> create(const std::string& tag) const;
```

Creates and returns a new component instance for the tag.

Returns `nullptr` when the tag is not registered.

```cpp
const auto component = registry.create("TodoItem");
```

### `contains()`

```cpp
bool contains(const std::string& tag) const;
```

Returns whether a factory is currently registered for the exact tag string.

Tag matching is case-sensitive.

### `tags()`

```cpp
std::vector<std::string> tags() const;
```

Returns registered tags sorted lexicographically.

---

## Diagnostics

### `diagnostics()`

```cpp
const Diagnostics& diagnostics() const;
```

Returns registration diagnostics accumulated by the registry.

Currently, duplicate tag registration produces a warning but does not reject the replacement.

### `clearDiagnostics()`

```cpp
void clearDiagnostics();
```

Clears accumulated registration diagnostics. It does not remove component factories.

---

## Template Tag Behavior

The component renderer recognizes tag names whose first character is uppercase.

```html
<TodoItem />
<PortalSelect id="role">...</PortalSelect>
```

For registered tags:

- a new component is created;
- quoted attributes are evaluated against the parent context;
- resulting values are supplied as `std::string` inputs;
- a child `RenderContext` is created;
- the full component lifecycle is executed.

For an unknown tag, normal rendering leaves the original tag in the output. Diagnostic rendering may report warning `DGL-CMP-002`.

---

## Application Access

Prefer high-level registration through `App`:

```cpp
app.component<TodoItemComponent>();
app.component<AlertComponent>("Alert");
```

Direct access is available from application services:

```cpp
auto& registry = app.services().components();
```

---

## Thread Safety

`ComponentRegistry` has no internal synchronization.

Complete registrations during application setup, before concurrent request processing begins. Concurrent reads are only safe when no thread mutates the registry or its diagnostics.

---

## Related Types

- [`Component`](component.md)
- [`TemplateComponent`](template-component.md)
- [`App`](../application/app.md)
- [`ApplicationServices`](../dependency-injection/application-services.md)
- `Diagnostics` *(coming soon)*
- [`Rendering`](../rendering/README.md)
