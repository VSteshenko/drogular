# Rendering

Rendering connects pages, components, templates, request data, application services, localization, and GraphQL data.

`RenderContext` is the central request-scoped object passed through the rendering pipeline. It stores typed template values and exposes the services and request metadata needed while producing HTML.

---

## Types

- [`RenderContext`](render-context.md) — request-scoped rendering data, services, request access, route parameters, localization, and GraphQL result storage.

Additional rendering types will be documented as the reference expands:

- `GraphQLResult`
- `RenderContextError`
- `TemplateRenderable`
- `CompiledTemplate`

---

## Role in Drogular

```text
HTTP GET request
      │
      ▼
    Router
      │
      ├── creates root RenderContext
      ├── attaches ApplicationServices
      ├── attaches HttpRequest
      └── stores route parameters
      │
      ▼
     Page
      │
      ├── onInit(context)
      └── render(context)
              │
              ▼
       Template / Components
              │
              └── child RenderContexts
```

The root context exists for one page request. Template loops and component expansion may create child contexts so local values can be added without modifying the parent context.

---

## Related Documentation

### Getting Started

- [Routing](../../getting-started/routing.md)
- [Components](../../getting-started/components.md)

### Cookbook

- [Localization](../../cookbook/localization.md)
- [GraphQL](../../cookbook/graphql.md)
- [State Management](../../cookbook/state-management.md)
