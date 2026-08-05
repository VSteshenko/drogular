# FormValidator

**Namespace:** `drogular`  
**Header:** `<drogular/form_validator.hpp>`  
**Kind:** Class

## Purpose

`FormValidator` builds a sequence of validation rules for form values stored in an `ActionContext` and evaluates them into a `ValidationResult`.

A validator is tied to one action context. Rule methods return the validator by reference so they can be chained.

---

## Synopsis

```cpp
class FormValidator {
public:
    explicit FormValidator(
        const ActionContext& context
    );

    FormValidator& required(std::string field);

    FormValidator& minLength(
        std::string field,
        size_t length
    );

    FormValidator& maxLength(
        std::string field,
        size_t length
    );

    FormValidator& email(std::string field);

    ValidationResult validate() const;
};
```

---

## Construction

```cpp
drogular::FormValidator validator(context);
```

The referenced `ActionContext` must remain alive while the validator is used.

---

## Public API

### `required()`

```cpp
FormValidator& required(std::string field);
```

Adds a rule that fails when the form value is missing or empty.

Generated message:

```text
<field> is required
```

### `minLength()`

```cpp
FormValidator& minLength(
    std::string field,
    size_t length
);
```

Adds a rule that fails when the form value is missing or contains fewer than `length` characters.

Because a missing value fails this rule, `minLength()` also implies that the field must be present. Add `required()` as well when you want a separate required-field error and explicit intent.

Generated message:

```text
<field> must be at least <length> characters
```

### `maxLength()`

```cpp
FormValidator& maxLength(
    std::string field,
    size_t length
);
```

Adds a rule that fails when a present value contains more than `length` characters.

A missing value passes this rule.

Generated message:

```text
<field> must be at most <length> characters
```

### `email()`

```cpp
FormValidator& email(std::string field);
```

Adds a basic email-shape check.

A missing or empty value passes this rule. Combine it with `required()` when the email address must be supplied.

The current check requires:

- text before `@`;
- text after `@`;
- a `.` after `@`;
- text after the final checked `.`.

Generated message:

```text
<field> is not a valid email
```

### `validate()`

```cpp
ValidationResult validate() const;
```

Evaluates rules in registration order and returns every generated validation error.

Validation does not modify the action context and does not throw for ordinary rule failures.

---

## Behavior

Rules are evaluated independently. A field can therefore produce more than one error.

For example, a missing field with both `required()` and `minLength()` produces both messages:

```cpp
const auto result =
    drogular::FormValidator(context)
        .required("title")
        .minLength("title", 2)
        .validate();
```

The validator stores field names exactly as provided and reads their values through `ActionContext::formValue()`.

---

## Example

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

repository->create(
    context.requireForm<std::string>("name"),
    context.requireForm<std::string>("email")
);
```

This pattern is used by `examples/repository_sample`.

---

## Related Types

- [`ValidationResult`](validation-result.md)
- [`ValidationError`](validation-error.md)
- `ActionContext`
- [`ActionValidationError`](action-validation-error.md)

## See Also

- [Cookbook: Forms & Validation](../../cookbook/forms-and-validation.md)
