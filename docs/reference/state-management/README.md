# State Management

The state management API provides a small observable value container through `State<T>`.

`State<T>` stores a mutable value and can notify registered C++ callbacks when `set()` publishes a replacement value. Applications commonly place state values inside application-specific stores and share those stores through dependency injection.

---

## Types

- [`State<T>`](state.md) — stores a value and notifies subscribers after `set()`.

---

## Typical Flow

```text
Page or Action
      │
      ▼
Application Store
      │
      ▼
State<T>::set()
      │
      ▼
C++ Subscribers
```

`State<T>` does not trigger a browser-side render. In server-rendered applications, a later request or redirect renders the updated store value.

---

## Cookbook

- [State Management](../../cookbook/state-management.md)

## Related API

- [`ApplicationServices`](../dependency-injection/application-services.md)
- [`RenderContext`](../rendering/render-context.md)
- `ActionContext` *(Actions reference coming soon)*
