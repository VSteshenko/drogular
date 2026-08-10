# State Management

## Problem

**Need to share application state between pages, actions, and components?**

This guide shows how to keep shared data in an application store instead of copying it across multiple parts of the application.

---

## Recommended Solution

Create an application-specific store, keep its mutable data behind explicit store operations, and register the store with the lifetime required by the application.

For simple in-memory examples, a singleton store lets pages and actions share the same data. In a multi-threaded server, the store must synchronize its own mutable state or delegate persistence to a thread-safe repository/database. `State<T>` itself does not provide synchronization.

Pages read from the store through `RenderContext`. Actions resolve the same store through `ActionContext` and apply explicit state changes.

---

## How It Works

### State Values

`State<T>` stores a mutable value and exposes it through `value()`.

```cpp
class TodoStore {
public:
    drogular::State<std::vector<Todo>> todos;
};
```

Read the current value without copying ownership into the page or component.

```cpp
const auto& currentTodos = todos.value();
```

Replace the value through `set()` when the state changes.

```cpp
todos.set(std::move(updatedTodos));
```

`State<T>` also supports subscribers that are notified after `set()` replaces the stored value.

```cpp
state.subscribe(
    [](const auto& value) {
        // React to the new value.
    }
);
```

The store decides when a change is complete and publishes the new value once.

---

### Application Store

The store owns shared application data and exposes operations that describe valid state changes.

TodoPWA keeps its todo collection in `TodoStore`:

```cpp
class TodoStore {
public:
    explicit TodoStore(std::vector<Todo> initialTodos)
        : todos(std::move(initialTodos)) {
        updateNextId();
    }

    void create(std::string title);
    void toggle(int id);
    void remove(int id);

    drogular::PagedResult<Todo> find(
        const TodoQuery& query
    ) const;

    drogular::State<std::vector<Todo>> todos;

private:
    int nextId_ = 1;
};
```

Callers do not need to know how todo identifiers are assigned, how items are updated, or how query results are paginated.

---

### Registering the Store

TodoPWA registers the store as a singleton so pages and actions resolve the same demonstration instance.

```cpp
app.services().add<TodoStore>(
    drogular::ServiceLifetime::Singleton
);
```

A transient store would create a separate state container for each resolution and would not preserve shared application data.

> **Concurrency:** singleton lifetime does not make a service thread-safe. Drogular may execute requests concurrently. Protect mutable in-memory state with appropriate synchronization, or use a persistence layer that already provides the required concurrency guarantees.

---

### Reading State from a Page

Pages resolve the store through `RenderContext`.

```cpp
const auto store =
    context.requireService<TodoStore>();

const auto result =
    store->find(query);
```

The page converts the result into template data but does not keep another authoritative copy of the todo collection.

```text
TodoStore
    │
    ▼
TodoPage::onInit
    │
    ▼
Template Context
    │
    ▼
Rendered HTML
```

---

### Updating State from an Action

Actions resolve the same singleton store through `ActionContext`.

```cpp
auto store =
    context.requireService<TodoStore>();

store->toggle(
    context.requireForm<int>("id")
);

return drogular::ActionResult::redirect("/");
```

The redirect starts a new request. The page then renders from the updated store state.

```text
Form Submission
      │
      ▼
Action Handler
      │
      ▼
TodoStore Update
      │
      ▼
Redirect
      │
      ▼
Page Reads Updated State
```

This is a server-rendered request cycle. `State<T>::set()` notifies C++ subscribers, but it does not by itself trigger a browser-side re-render.

---

## Example

TodoPWA creates a new item by copying the current collection, applying the change, and publishing the replacement value. The sample below demonstrates the state-update pattern; a production singleton store must additionally synchronize concurrent readers and writers.

```cpp
void create(std::string title) {
    if (title.empty()) {
        return;
    }

    auto updatedTodos = todos.value();

    updatedTodos.push_back({
        nextId_++,
        std::move(title),
        false
    });

    todos.set(std::move(updatedTodos));
}
```

`CreateTodoAction` validates the request and delegates the state change to the store.

```cpp
const auto validation =
    drogular::FormValidator(context)
        .required("title")
        .minLength("title", 2)
        .maxLength("title", 100)
        .validate();

if (!validation.valid()) {
    return drogular::ActionResult::redirect("/");
}

auto store =
    context.requireService<TodoStore>();

store->create(
    context.requireForm<std::string>("title")
);

return drogular::ActionResult::redirect("/");
```

The page receives the updated collection on the redirected request.

---

## Best Practices

- Keep one authoritative owner for each piece of shared state.
- Choose service lifetime deliberately; use singleton only when shared ownership is required.
- Make singleton stores thread-safe when they contain mutable in-memory state.
- Expose explicit store operations instead of mutable containers.
- Publish completed changes through `State<T>::set()`.
- Read shared state through services rather than copying it into pages or components.
- Keep request validation in actions and state transitions in stores.
- Do not assume that `State<T>` automatically updates browser-rendered HTML.

---

## See Also

### Getting Started

- [Dependency Injection](../getting-started/dependency-injection.md)
- [Components](../getting-started/components.md)

### API Reference

- [`State<T>`](../reference/state-management/state.md)
- [`ApplicationServices`](../reference/dependency-injection/application-services.md)
- [`RenderContext`](../reference/rendering/render-context.md)
- [`ActionContext`](../reference/actions/action-context.md)
