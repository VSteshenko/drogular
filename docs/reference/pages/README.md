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
    ├── calls Page::onInit()
    └── calls Page::render()
            │
            ▼
        HTML response
```

`App::page<PageType>()` creates one page instance during route registration. The router reuses that instance for later requests.

Pages should therefore keep request-specific data in `RenderContext`, not in page data members.

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

- [App](../application/app.md)
- Rendering *(coming soon)*
- Components *(coming soon)*
- Authentication and Sessions *(coming soon)*
