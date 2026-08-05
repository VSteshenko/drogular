# ActionValidationError

**Namespace:** `drogular`  
**Header:** `<drogular/action_validation_error.hpp>`  
**Kind:** Exception class  
**Base class:** `std::runtime_error`

## Purpose

`ActionValidationError` reports that a required action input value is missing or cannot be converted to the requested type.

`ActionContext::requireForm<T>()` throws this exception when required typed form access fails.

---

## Synopsis

```cpp
class ActionValidationError
    : public std::runtime_error
{
public:
    explicit ActionValidationError(
        std::string message
    );
};
```

---

## Construction

```cpp
throw drogular::ActionValidationError(
    "Invalid input"
);
```

The message is available through the inherited `what()` member.

---

## Behavior

For `ActionContext::requireForm<T>(name)`, the generated message is:

```text
Invalid or missing form value: <name>
```

Supported form conversion types currently include:

- `std::string`
- `int`
- `double`
- `bool`

For `int` and `double`, conversion failure causes `requireForm<T>()` to throw. For `bool`, the values `true`, `1`, and `on` map to `true`; other present strings map to `false`.

Ordinary validation-rule failures should normally be handled through `ValidationResult` before required values are read.

---

## Example

```cpp
const auto validation =
    drogular::FormValidator(context)
        .required("title")
        .validate();

if (!validation.valid()) {
    return drogular::ActionResult::redirect("/");
}

const auto title =
    context.requireForm<std::string>("title");
```

The validation check prevents the expected missing-value case from reaching the throwing accessor.

---

## Related Types

- [`FormValidator`](form-validator.md)
- [`ValidationResult`](validation-result.md)
- `ActionContext`
