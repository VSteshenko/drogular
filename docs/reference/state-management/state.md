# `State<T>`

**Namespace:** `drogular`  
**Header:** `<drogular/state.hpp>`  
**Kind:** Class template

## Purpose

`State<T>` stores one mutable value and maintains a list of callbacks that are invoked after `set()` replaces that value.

It is a low-level building block. Drogular applications typically embed one or more state values inside an application-specific store rather than resolving `State<T>` directly throughout the application.

---

## Synopsis

```cpp
template <typename T>
class State {
public:
    using Callback = std::function<void(const T&)>;

    State();
    explicit State(T value);

    const T& value() const;
    T& value();

    void set(T value);
    void subscribe(Callback callback);
};
```

---

## Template Parameters

### `T`

The stored value type.

A default-constructed `State<T>` requires `T` to be default-constructible. Calling `set()` requires the supplied value to be movable or copyable into the state.

---

## Construction

### Default construction

```cpp
drogular::State<int> state;
```

Initializes the stored value with `T{}`.

### Value construction

```cpp
drogular::State<int> state(42);
```

Moves or copies the supplied value into the state.

Construction does not notify subscribers.

---

## Public API

### `Callback`

```cpp
using Callback = std::function<void(const T&)>;
```

Callback type used by `subscribe()`.

The callback receives a const reference to the current stored value during notification.

### `value() const`

```cpp
const T& value() const;
```

Returns a const reference to the stored value.

No copy is created.

### `value()`

```cpp
T& value();
```

Returns a mutable reference to the stored value.

Mutating the returned object does **not** notify subscribers:

```cpp
drogular::State<std::vector<int>> state;

state.value().push_back(1); // No callback is invoked.
```

Use `set()` when a completed change must be published to subscribers.

### `set()`

```cpp
void set(T value);
```

Replaces the stored value, then invokes every registered callback with the new value.

Callbacks are invoked on every call to `set()`. `State<T>` does not compare the previous and new values.

```cpp
state.set(42);
```

The value is assigned before callbacks run, so callbacks can read the new value through `value()`.

### `subscribe()`

```cpp
void subscribe(Callback callback);
```

Adds a callback to the subscriber list.

Subscription does not invoke the callback immediately. The callback runs only after a later call to `set()`.

The current API does not return a subscription handle and does not provide an unsubscribe operation.

---

## Behavior

Subscribers are stored in registration order and are invoked synchronously from `set()`.

```cpp
drogular::State<int> state(0);

state.subscribe([](const int& value) {
    // Runs when set() is called.
});

state.set(1);
```

Important behavior:

- direct mutation through mutable `value()` does not notify;
- every `set()` call notifies, even when the assigned value is equal to the current value;
- subscription does not replay the current value;
- callbacks remain registered for the lifetime of the `State<T>` instance;
- exceptions thrown by a callback propagate from `set()`;
- notification is a C++ callback mechanism and does not trigger browser-side rendering.

---

## Lifetime and Thread Safety

`State<T>` does not own an external execution context. Its value and callbacks live as long as the state object itself.

When state is embedded in a dependency-injected store, the store lifetime determines how long the state persists. TodoPWA registers `TodoStore` as a singleton so pages and actions resolve the same state across requests.

`State<T>` does not perform synchronization. Concurrent calls to `value()`, `set()`, or `subscribe()` require external synchronization.

Callbacks must not outlive objects they capture. Because there is no unsubscribe operation, avoid capturing short-lived objects when the state has a longer lifetime.

---

## Example

TodoPWA stores its collection in an application-specific store:

```cpp
class TodoStore {
public:
    explicit TodoStore(std::vector<Todo> initialTodos)
        : todos(std::move(initialTodos)) {
        updateNextId();
    }

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

    drogular::State<std::vector<Todo>> todos;

private:
    void updateNextId();
    int nextId_ = 1;
};
```

The store copies the current collection, completes the update, and publishes the replacement value once through `set()`.

A subscriber can observe later updates:

```cpp
drogular::State<int> counter(0);

counter.subscribe(
    [](const int& value) {
        std::cout << value << '\n';
    }
);

counter.set(1);
counter.set(2);
```

---

## Related Types

- `ApplicationServices`
- `RenderContext`
- `ActionContext`

## See Also

- [Cookbook: State Management](../../cookbook/state-management.md)
