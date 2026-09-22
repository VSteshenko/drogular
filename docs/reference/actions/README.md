# Actions

The Actions API handles request-oriented endpoints that return `ActionResult`. POST actions are the normal command/mutation path, while GET actions are available for read-only endpoints that need the Action API rather than Page rendering.

An action receives an `ActionContext`, reads request data or application services, performs application logic, and returns an `ActionResult`. The router converts that result into a Drogon HTTP response.

---

## Types

- [`ActionHandler`](action-handler.md) — base class for application action handlers.
- [`ActionContext`](action-context.md) — request and service context passed to an action.
- [`ActionResult`](action-result.md) — describes the response produced by an action.
- [`ActionRenderer`](action-renderer.md) — renders a Component into an HTML `ActionResult` while preserving request-bound context.
- [`ActionResultType`](action-result-type.md) — identifies the response variant stored in an `ActionResult`.
- [`Cookie`](cookie.md) — cookie metadata and security attributes attached to an action result.
- [`CookieOptions`](cookie.md) — options for Path, HttpOnly, Secure, SameSite, and Max-Age.
- [`FileResponseInfo`](file-response-info.md) — file response metadata.
- [`toHttpResponse()`](action-response.md) — converts an `ActionResult` into a Drogon response.
- [`ActionValidationError`](../forms-and-validation/action-validation-error.md) — reports missing or invalid required action input.
- [`Action Error Handling`](error-handling.md) — defines exception hierarchy and action exception-to-HTTP mapping.

---

## Typical Flow

```text
GET / POST Request
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

Register POST actions with `App::action<ActionType>()`. Register read-only GET actions with `App::get<ActionType>()`.

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
