# Action Error Handling

## Purpose

Drogular defines a small exception-to-HTTP contract for action routes. It keeps expected input failures distinguishable from unexpected framework or application failures while preventing internal exception messages from leaking through `500` responses.

---

## Exception Hierarchy

Framework-specific exceptions derive from `DrogularError`, which derives from `std::runtime_error`. Specialized types include `ActionValidationError`, `ActionContextError`, `RenderContextError`, and `GraphQLClientError`.

```text
std::exception
    └── std::runtime_error
          └── DrogularError
                ├── ActionValidationError
                ├── ActionContextError
                ├── RenderContextError
                └── GraphQLClientError
```

The hierarchy lets application code catch framework errors as a group while preserving specialized contracts where they matter.

---

## Action-to-HTTP Mapping

When an exception escapes `ActionHandler::handle()`, the router applies these rules:

| Exception | HTTP status | Response body |
| --- | --- | --- |
| `ActionValidationError` | `400 Bad Request` | `what()` validation message |
| other `std::exception` | `500 Internal Server Error` | `Internal Server Error` |
| unknown exception | `500 Internal Server Error` | `Internal Server Error` |

Unexpected exception messages are logged by the router but are not exposed to the client.

---

## Validation vs Exceptions

Use `ValidationResult` when the application needs structured field-level feedback or a custom response. Use required accessors such as `ActionContext::requireForm<T>()` when invalid or missing input should stop action execution and use the standard `400` response.

```cpp
const auto title = context.requireForm<std::string>("title");
```

If `title` is missing or invalid, `ActionValidationError` is thrown and translated by the router.

Errors such as a missing required service are `ActionContextError` values. They are framework/application failures rather than client validation failures, so they map to the safe `500` response.

---

## Related Types

- [`ActionHandler`](action-handler.md)
- [`ActionContext`](action-context.md)
- [`ActionValidationError`](../forms-and-validation/action-validation-error.md)
- [`toHttpResponse()` / `toHttpErrorResponse()`](action-response.md)
- [`Router`](../routing/router.md)
