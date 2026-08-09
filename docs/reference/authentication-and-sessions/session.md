# `Session`

**Namespace:** `drogular`  
**Header:** `<drogular/session.hpp>`  
**Kind:** Class

## Purpose

`Session` stores string key/value data associated with one session id.

## Public API

### `set()`

```cpp
void set(
    std::string key,
    std::string value
);
```

Adds or replaces a value.

### `get()`

```cpp
std::optional<std::string> get(
    const std::string& key
) const;
```

Returns the stored value or `std::nullopt` when the key is absent.

### `has()`

```cpp
bool has(
    const std::string& key
) const;
```

Returns whether a key exists.

### `remove()`

```cpp
void remove(
    const std::string& key
);
```

Removes a key when present.

### `clear()`

```cpp
void clear();
```

Removes all values from the session.

## Framework-Managed `_id`

[`SessionStore::create()`](session-store.md) stores the generated session identifier in the session under the key `_id`:

```cpp
const auto id = session->get("_id");
```

`Session` itself does not reserve or protect this key. Application code can overwrite or remove it.

## Example

```cpp
auto session = context.session();

session->set("user_id", "42");
session->set("username", "alice");
session->set("role", "admin");
```

## Behavior and Lifetime

`Session` contains only in-memory strings. It has no timestamps, expiry policy, serialization, or persistence behavior.

The lifetime of a session object is controlled by `std::shared_ptr` ownership and by whether it remains stored in [`SessionStore`](session-store.md).

## Thread Safety

`Session` synchronizes access to its internal key/value map. Individual `set()`, `get()`, `has()`, `remove()`, and `clear()` calls may be used concurrently from request-processing threads.

Sequences of multiple calls are not automatically atomic. Application logic that performs a read-modify-write operation across several calls still needs its own coordination when atomicity matters.

## Related Types

- [`SessionStore`](session-store.md)
- [`AuthSupport`](auth-support.md)
- [`ActionContext`](../actions/action-context.md)
