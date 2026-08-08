# Routing

Routing connects incoming HTTP requests to Drogular pages, actions, static files, and the application service worker.

The Routing API provides the application router, route-pattern matching with named parameters, and URL query-string encoding helpers.

---

## Types

- [`Router`](router.md) — connects Drogular handlers to Drogon routes.
- [`RoutePattern`](route-pattern.md) — matches path segments and extracts named parameters.
- [`Url`](url.md) — provides percent-encoding helpers for query-string values.

---

## Typical Flow

```text
HTTP Request
     │
     ▼
   Drogon
     │
     ▼
   Router
     │
     ├── Page route ──────► RenderContext ──────► Page
     │
     ├── Action route ────► ActionContext ──────► ActionHandler
     │
     ├── Static files ────► StaticFileResponse
     │
     └── Service worker ──► StaticFileResponse
```

Parameterized page and action routes use `RoutePattern` to extract values such as `{id}` and place them into the request context.

Routes are normally registered through [`App`](../application/app.md) rather than by constructing a `Router` directly.

---

## Getting Started

- [Routing](../../getting-started/routing.md)
- [Your First Drogular Application](../../getting-started/your-first-drogular-application.md)

## Related Reference

- [App](../application/app.md)
- [Page](../pages/page.md)
- [ActionHandler](../actions/action-handler.md)
- [RenderContext](../rendering/render-context.md)
