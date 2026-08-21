# Rendering

Rendering connects pages, components, templates, request data, application services, localization, and GraphQL data.

`RenderContext` is the central request-scoped object passed through the rendering pipeline. It stores typed template values and exposes the services and request metadata needed while producing HTML.

---

## Types

- [`RenderContext`](render-context.md) — request-scoped rendering data, services, request access, route parameters, localization, and GraphQL result storage.
- [`GraphQLResult`](graphql-result.md) — stores typed GraphQL-derived values associated with a render context.
- [`TemplateRenderable`](template-renderable.md) — shared template pipeline used by template-backed pages and components.
- [`template_compiler::CompiledTemplate`](compiled-template.md) — low-level compiled-template representation and compilation entry points.
- [Template Conditional Expressions](conditional-expressions.md) — `@if(...)` comparisons, logical operators, literals, grouping, and diagnostics.

`RenderContextError` is documented as part of [`RenderContext`](render-context.md) and the framework error contract.

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
