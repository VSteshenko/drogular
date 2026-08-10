# Forms & Validation

## Problem

**Need to validate user input?**

This guide shows how to validate submitted form values before application state or persistent data is modified.

---

## Recommended Solution

Validate request data inside the action with `FormValidator`, inspect the resulting `ValidationResult`, and only read required typed values after validation succeeds.

This keeps request handling explicit and prevents invalid input from reaching stores, repositories, or services.

---

## How It Works

### FormValidator

`FormValidator` reads form values from `ActionContext` and evaluates a chain of validation rules.

The TodoPWA create action requires a title between 2 and 100 characters:

```cpp
const auto validation =
    drogular::FormValidator(context)
        .required("title")
        .minLength("title", 2)
        .maxLength("title", 100)
        .validate();
```

Rules are evaluated when `validate()` is called.

The current validator supports:

- `required()`
- `minLength()`
- `maxLength()`
- `email()`

`maxLength()` and `email()` skip missing values. `minLength()` treats a missing value as an error. Combine format rules with `required()` when presence should be explicit and produce a dedicated required-field error.

```cpp
const auto validation =
    drogular::FormValidator(context)
        .required("email")
        .email("email")
        .validate();
```

---

### ValidationResult

`validate()` returns a `ValidationResult`.

```cpp
if (!validation.valid()) {
    return drogular::ActionResult::redirect("/");
}
```

Each validation error contains the field name and its message.

```cpp
for (const auto& error : validation.errors()) {
    std::cout
        << error.field
        << ": "
        << error.message
        << '\n';
}
```

Field-specific helpers are available when one input must be inspected directly.

```cpp
if (validation.hasError("email")) {
    const auto message =
        validation.error("email");
}
```

`ValidationResult` stores errors only. The application decides whether to redirect, render a form again, or expose the messages through another response model.

---

### Reading Validated Values

After validation succeeds, use `requireForm<T>()` to read the value in the required type.

```cpp
const auto title =
    context.requireForm<std::string>("title");
```

`requireForm<T>()` throws `ActionValidationError` when a required value is missing or cannot be converted. The action router translates that expected exception into `400 Bad Request`.

Validate first, then use required accessors for the values needed by the operation. Use `ValidationResult` when the UI needs field-specific feedback; use `ActionValidationError` for required typed values that make the request invalid as a whole.

### Typed Parsing Rules

Typed form access is strict. The whole submitted value must match the requested type.

```text
"42"       -> int 42
"42abc"    -> invalid
"3.14"     -> double 3.14
"3.14x"    -> invalid
"true"     -> bool true
"1"        -> bool true
"on"       -> bool true
"false"    -> bool false
"0"        -> bool false
"off"      -> bool false
"yes"      -> invalid
```

Use `form<T>()` when invalid or missing input should be represented as `std::nullopt`. Use `requireForm<T>()` when the action cannot continue without a valid value.

---

## Example

TodoPWA validates the title before updating its shared store.

```cpp
class CreateTodoAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
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
    }
};
```

The request follows this path:

```text
POST Form
    │
    ▼
ActionContext
    │
    ▼
FormValidator
    │
    ├── invalid ──► redirect
    │
    ▼ valid
requireForm<T>()
    │
    ▼
TodoStore
    │
    ▼
redirect
```

The repository sample uses the same approach for multiple fields and email validation:

```cpp
const auto validation =
    drogular::FormValidator(context)
        .required("name")
        .minLength("name", 2)
        .required("email")
        .email("email")
        .validate();

if (!validation.valid()) {
    return drogular::ActionResult::redirect("/users");
}

auto repository =
    context.requireService<
        RepositorySampleUserRepository
    >();

repository->create(
    context.requireForm<std::string>("name"),
    context.requireForm<std::string>("email")
);
```

Validation happens before the repository is called, so invalid input cannot modify persistent data.

---

## Best Practices

- Validate form values before modifying stores, repositories, or services.
- Keep validation rules close to the action that owns the operation.
- Chain `required()` with format or length rules when a value must be present.
- Read required typed values only after validation succeeds.
- Use `ValidationResult` for recoverable, field-specific form feedback.
- Use `ActionValidationError` for invalid required typed input that should become `400 Bad Request`.

---

## See Also

### Getting Started

- [Actions & Routing](../getting-started/routing.md)
- [Dependency Injection](../getting-started/dependency-injection.md)

### API Reference

- [`FormValidator`](../reference/forms-and-validation/form-validator.md)
- [`ValidationResult`](../reference/forms-and-validation/validation-result.md)
- [`ValidationError`](../reference/forms-and-validation/validation-error.md)
- [`ActionContext`](../reference/actions/action-context.md)
- [`ActionValidationError`](../reference/forms-and-validation/action-validation-error.md)
- [`Action Error Handling`](../reference/actions/error-handling.md)
