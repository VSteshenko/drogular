# Actions

The Actions API handles application commands submitted through POST routes.

An action receives an `ActionContext`, reads request data or application services, performs application logic, and returns an `ActionResult`. The router converts that result into a Drogon HTTP response.

---

## Types

- [`ActionHandler`](action-handler.md) — base class for application action handlers.
- [`ActionContext`](action-context.md) — request and service context passed to an action.
- [`ActionResult`](action-result.md) — describes the response produced by an action.
- [`ActionResultType`](action-result-type.md) — identifies the response variant stored in an `ActionResult`.
- [`Cookie`](cookie.md) — cookie metadata attached to an action result.
- [`FileResponseInfo`](file-response-info.md) — file response metadata.
- [`toHttpResponse()`](action-response.md) — converts an `ActionResult` into a Drogon response.
- [`ActionValidationError`](../forms-and-validation/action-validation-error.md) — reports missing or invalid required action input.

---

## Typical Flow

```text
POST Request
     │
     ▼
   Router
     │
     ▼
ActionContext
     │
     ▼
ActionHandler::handle()
     │
     ├── Form data
     ├── Route parameters
     ├── Cookies / Session
     └── Application services
     │
     ▼
ActionResult
     │
     ▼
toHttpResponse()
     │
     ▼
HTTP Response
```

Actions are registered through `App::action<ActionType>()` and currently accept POST requests only.

---

## Getting Started

- [Actions & Routing](../../getting-started/routing.md)
- [Dependency Injection](../../getting-started/dependency-injection.md)

## Cookbook

- [Forms & Validation](../../cookbook/forms-and-validation.md)
- [State Management](../../cookbook/state-management.md)

---

## Related Reference

- [Router](../routing/router.md)
