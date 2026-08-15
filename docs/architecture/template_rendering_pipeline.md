# Template Rendering Pipeline

Drogular uses one rendering pipeline for template-backed pages and components.

## Public types

Application code continues to derive from:

- `drogular::TemplatePage` for route-mounted pages;
- `drogular::TemplateComponent` for reusable UI components.

Both types share the `drogular::TemplateRenderable` implementation internally.
This keeps the public API explicit while preventing the two renderers from
drifting apart.

## Pipeline

Every template-backed render executes the same stages:

1. Resolve `templatePath()` or use `templateHtml()`.
2. Resolve the source through `TemplateSourceCache` when enabled.
3. Compose `layoutPath()` around `@content` when a layout is provided.
4. Process `@include` directives.
5. Compile the template into the AST and reuse `TemplateCache`.
6. Render expressions, conditions and loops against `RenderContext`.
7. Expand registered component tags recursively.

`TemplateComponent` applies its inputs to the component render context before
entering this pipeline. `TemplatePage` enters the pipeline directly.

## Compatibility

The consolidation does not require application migrations:

- existing `templateHtml()` overrides remain valid;
- existing `templatePath()` overrides remain valid;
- existing page `layoutPath()` overrides remain valid;
- `TemplatePage` and `TemplateComponent` names and inheritance roles remain
  unchanged.

The shared base is intentionally an implementation-level abstraction. Pages
and components remain separate concepts because pages are routable and may
provide GraphQL queries, while components own inputs, slots, children and
component lifecycle behavior.

## Component API review

The lifecycle contract remains:

```text
onInit -> render -> onDestroy
```

No additional hooks were added. The existing lifecycle covers current Portal
Demo, TodoPWA and sample requirements, and adding hooks without proven use
would increase the stable API surface before 1.0.


## Lifetime and caching

`TemplatePage` instances are created per request. Its instance-owned compiled-template cache therefore has the same lifetime as that Page instance. Reusable template **source** caching is application-level and is provided through `ApplicationServices::templateSourceCache()` when template caching is enabled.

`TemplateComponent` instances created from the component registry are created for each component expansion. They use the same rendering pipeline but remain distinct component objects with their own lifecycle.

See [Request Lifecycle](request-lifecycle.md) for the ownership model around Pages, child render contexts, services, and component lifecycle.
