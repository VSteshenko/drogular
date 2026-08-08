# `DependencyValidationResult`

**Namespace:** `drogular`  
**Header:** `<drogular/services.hpp>`  
**Kind:** Class

## Purpose

`DependencyValidationResult` stores errors produced by `ApplicationServices::validateDependencies()`.

A newly constructed result is valid. Adding any error makes it invalid.

---

## Synopsis

```cpp
class DependencyValidationResult {
public:
    void addError(std::string error);
    bool valid() const;
    const std::vector<std::string>& errors() const;
};
```

---

## Public API

### `addError()`

```cpp
void addError(std::string error);
```

Appends an error string.

### `valid()`

```cpp
bool valid() const;
```

Returns `true` when no errors have been added.

### `errors()`

```cpp
const std::vector<std::string>& errors() const;
```

Returns all validation errors in insertion order.

---

## Current Validation Errors

`ApplicationServices::validateDependencies()` currently emits generic messages:

```text
Service dependency is not registered
Circular dependency detected
```

The messages do not currently identify the involved service types.

---

## Example

```cpp
const auto validation =
    app.services().validateDependencies();

if (!validation.valid()) {
    for (const auto& error : validation.errors()) {
        std::cerr << error << '\n';
    }
}
```

---

## Related Types

- [`ApplicationServices`](application-services.md)
- [`DependencyGraph`](dependency-graph.md)
