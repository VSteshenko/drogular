# `ActionRenderer`

**Namespace:** `drogular`  
**Header:** `<drogular/action_renderer.hpp>`  
**Kind:** Utility class

## Purpose

`ActionRenderer` renders a Drogular `Component` from inside an Action and returns the rendered HTML as an `ActionResult`.

It bridges `ActionContext` to the request's shared `RenderContext`, so fragment rendering uses the same request-bound services, route parameters, session state, localization data, and scoped service cache instead of creating an unrelated rendering request.

## Basic usage

```cpp
#include <drogular/action_renderer.hpp>

return drogular::ActionRenderer::render<ProjectFormFragment>(context);
```

The component is default-constructed and rendered through the normal Component lifecycle.

## Supplying render data

```cpp
return drogular::ActionRenderer::render<ProjectFormFragment>(
    context,
    [&](drogular::RenderContext& renderContext) {
        renderContext.set("project", project);
        renderContext.set("errors", errors);
    }
);
```

The setup callback receives `RenderContext&` and runs before the component is rendered.

## HTTP status

```cpp
return drogular::ActionRenderer::render<ProjectFormFragment>(
    context,
    [&](drogular::RenderContext& renderContext) {
        renderContext.set("errors", errors);
    },
    drogon::k400BadRequest
);
```

This is particularly useful with `dg-post`: Drogular Interactions can place the returned validation fragment into the configured target even when the response status is non-2xx.

## Request-context preservation

```text
ActionContext
     │
     ▼
shared request state
     │
     ▼
RenderContext
     │
     ▼
Component lifecycle
     │
     ▼
ActionResult::html(...)
```

The bridge is framework-owned. Application code should use `ActionRenderer` rather than constructing a separate `RenderContext` for an Action fragment.

## Requirements

`ComponentType` must derive from `drogular::Component` and be default-constructible. The setup callback, when supplied, must be invocable with `RenderContext&`.

## Related Documentation

- [ActionContext](action-context.md)
- [ActionResult](action-result.md)
- [Drogular Interactions](../interactions/README.md)
- [Server-driven interactions Cookbook](../../cookbook/server-driven-interactions.md)
