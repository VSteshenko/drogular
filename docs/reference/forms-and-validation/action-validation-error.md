# ActionValidationError

**Namespace:** `drogular`  
**Header:** `<drogular/action_validation_error.hpp>`  
**Kind:** Exception class  
**Base class:** `DrogularError`

## Purpose

`ActionValidationError` reports that a required action input value is missing or cannot be converted to the requested type.

`ActionContext::requireForm<T>()` throws this exception when required typed form access fails.

---

## Synopsis

```cpp
class ActionValidationError
    : public DrogularError
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

For `int` and `double`, the complete input string must be a valid value; partial numeric strings are rejected. For `bool`, `true`, `1`, and `on` map to `true`, while `false`, `0`, and `off` map to `false`. Any other present string is invalid.

When this exception escapes an action handler, the router converts it to `400 Bad Request` and uses the exception message as the plain-text response body. Other exceptions use the framework's safe `500` response contract.

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
- [`ActionContext`](../actions/action-context.md)
- [`Action Error Handling`](../actions/error-handling.md)
