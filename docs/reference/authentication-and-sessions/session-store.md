# `SessionStore`

**Namespace:** `drogular`  
**Header:** `<drogular/session_store.hpp>`  
**Kind:** Class

## Purpose

`SessionStore` owns the framework's default in-memory session collection.

It creates session identifiers and maps them to `Session` objects.

## Construction and Registration

Applications typically register one store as a singleton:

```cpp
app.services().add<drogular::SessionStore>(
    drogular::ServiceLifetime::Singleton
);
```

Authentication helpers resolve this service through dependency injection.

## Public API

### `create()`

```cpp
std::shared_ptr<Session> create();
```

Creates a session, generates a unique id relative to the current store, stores that id as `_id` inside the session, inserts the session into the store, and returns it.

### `get()`

```cpp
std::shared_ptr<Session> get(
    const std::string& id
);
```

Returns the stored session or `nullptr` when the id is unknown.

### `contains()`

```cpp
bool contains(
    const std::string& id
) const;
```

Returns whether an id exists.

### `remove()`

```cpp
void remove(
    const std::string& id
);
```

Removes the mapping for the id. Existing `shared_ptr<Session>` instances may keep the object alive after removal.

### `clear()`

```cpp
void clear();
```

Removes every stored session mapping.

## Session Id Generation

The current implementation generates ids from a thread-local `std::mt19937_64` value formatted as hexadecimal text and retries on collisions already present in the store.

This is an implementation detail, not a cryptographic session-token guarantee. Applications with stronger security requirements should not treat the current generator as a cryptographic identity primitive.

## Storage Model

The default store is process-local and memory-only:

```text
Application process
      │
      ▼
SessionStore
      │
      ├── id → Session
      ├── id → Session
      └── id → Session
```

It does not provide:

- database persistence;
- distributed session sharing;
- expiration or cleanup by age;
- capacity limits;
- cookie management.

## Example

```cpp
auto session = context.session();

session->set("username", "alice");

const auto sessionId =
    session->get("_id").value();

return drogular::ActionResult::redirect("/dashboard")
    .cookie("session_id", sessionId);
```

`ActionContext::session()` creates a session when none exists, but it does **not** attach the session id cookie to the response. Application code must do that explicitly.

## Thread Safety

`SessionStore` synchronizes access to its internal session map. `create()`, `get()`, `contains()`, `remove()`, and `clear()` may be called concurrently from request-processing threads.

The store protects its own container operations. Code that combines several store calls into a larger logical transaction must provide additional coordination if that sequence needs to be atomic.

## Related Types

- [`Session`](session.md)
- [`AuthSupport`](auth-support.md)
- [`ApplicationServices`](../dependency-injection/application-services.md)
- [`ActionContext`](../actions/action-context.md)
