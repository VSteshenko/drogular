# Pages

Pages are route-mounted components that produce complete HTML responses for GET requests.

The Pages API provides a base class for direct HTML rendering, a template-backed page implementation, and helpers for common page and PWA context values.

---

## Types

- [`Page`](page.md) — base class for route-mounted pages.
- [`TemplatePage`](template-page.md) — page base class backed by Drogular's template pipeline.
- [`PageSupport`](page-support.md) — adds common page and PWA values to a `RenderContext`.
- [`PwaPageSupport`](pwa-page-support.md) — adds PWA metadata and service-worker markup.

---

## Typical Flow

```text
GET request
    │
    ▼
Router
    │
    ├── creates RenderContext
    ├── attaches request and route parameters
    ├── creates a fresh Page instance
    └── runs onInit() -> render() -> onDestroy()
            │
            ▼
        HTML response
```

`App::page<PageType>()` registers a factory. The router invokes that factory for every matching request, so page instances are request-scoped rather than shared across requests.

`RenderContext` remains the normal place for request values that must flow into templates, child components, and scoped services.

---

## Registration

```cpp
app.page<HomePage>("/");
app.page<UserEditPage>("/users/{id}/edit");
```

The page type must inherit from `drogular::Page` and be default-constructible.

---

## Getting Started

- [Your First Drogular Application](../../getting-started/your-first-drogular-application.md)
- [Routing](../../getting-started/routing.md)
- [Project Structure](../../getting-started/project-structure.md)

## Related Reference

- [Router](../routing/router.md)
- [App](../application/app.md)
- [Rendering](../rendering/README.md)
- [Components](../components/README.md)
- [Authentication and Sessions](../authentication-and-sessions/README.md)
