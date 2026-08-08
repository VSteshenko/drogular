# Forms and Validation

The forms and validation API validates submitted form values before an action modifies application state or persistent data.

`FormValidator` reads values from [`ActionContext`](../actions/action-context.md), evaluates registered rules, and returns a `ValidationResult`. Required typed values can then be read through `ActionContext::requireForm<T>()`.

---

## Types

- [`FormValidator`](form-validator.md) — builds and executes form validation rules.
- [`ValidationResult`](validation-result.md) — stores validation errors and provides field-specific lookup.
- [`ValidationError`](validation-error.md) — represents one field validation error.
- [`ActionValidationError`](action-validation-error.md) — reports missing or invalid required action input.

---

## Typical Flow

```text
ActionContext
      │
      ▼
FormValidator
      │
      ▼
ValidationResult
      │
      ├── invalid ──► return an application response
      │
      ▼ valid
ActionContext::requireForm<T>()
      │
      ▼
Store / Repository / Service
```

Validation failures returned through `ValidationResult` are normal application control flow. `ActionValidationError` is thrown only when a required accessor cannot return the requested value.

---

## Cookbook

- [Forms & Validation](../../cookbook/forms-and-validation.md)

## Getting Started

- [Actions & Routing](../../getting-started/routing.md)
- [Dependency Injection](../../getting-started/dependency-injection.md)
