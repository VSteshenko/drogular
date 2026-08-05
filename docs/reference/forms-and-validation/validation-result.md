# ValidationResult

**Namespace:** `drogular`  
**Header:** `<drogular/validation_result.hpp>`  
**Kind:** Class

## Purpose

`ValidationResult` stores validation errors produced by `FormValidator` or application-defined validation code.

A newly constructed result is valid. Adding any error makes it invalid.

---

## Synopsis

```cpp
class ValidationResult {
public:
    void addError(
        std::string field,
        std::string message
    );

    bool valid() const;

    const std::vector<ValidationError>& errors() const;

    bool hasError(
        const std::string& field
    ) const;

    std::optional<std::string> error(
        const std::string& field
    ) const;
};
```

---

## Construction

```cpp
drogular::ValidationResult result;
```

The result initially contains no errors, so `valid()` returns `true`.

Most application code receives a result from `FormValidator::validate()` instead of constructing one directly.

---

## Public API

### `addError()`

```cpp
void addError(
    std::string field,
    std::string message
);
```

Appends one validation error.

Errors are retained in insertion order. Duplicate fields and duplicate messages are allowed.

### `valid()`

```cpp
bool valid() const;
```

Returns `true` when the error collection is empty.

### `errors()`

```cpp
const std::vector<ValidationError>& errors() const;
```

Returns a const reference to all validation errors in insertion order.

The reference remains valid while the `ValidationResult` exists and is not modified through `addError()` in a way that reallocates the underlying vector.

### `hasError()`

```cpp
bool hasError(
    const std::string& field
) const;
```

Returns `true` when at least one stored error has the exact field name.

### `error()`

```cpp
std::optional<std::string> error(
    const std::string& field
) const;
```

Returns the first error message stored for the exact field name.

Returns `std::nullopt` when the field has no error.

---

## Behavior

`ValidationResult` does not decide how failures are presented. The application can redirect, render the form again, serialize errors, or map them into another response model.

The type contains no success value and no exception state. Validity is determined only by whether errors have been added.

---

## Example

```cpp
const auto validation =
    drogular::FormValidator(context)
        .required("email")
        .email("email")
        .validate();

if (!validation.valid()) {
    for (const auto& error : validation.errors()) {
        std::cerr
            << error.field
            << ": "
            << error.message
            << '\n';
    }

    return drogular::ActionResult::redirect("/users");
}
```

Field-specific lookup:

```cpp
if (validation.hasError("email")) {
    const auto message = validation.error("email");
}
```

---

## Related Types

- [`ValidationError`](validation-error.md)
- [`FormValidator`](form-validator.md)

## See Also

- [Cookbook: Forms & Validation](../../cookbook/forms-and-validation.md)
