# Architecture

This section explains the architectural contracts and design decisions behind Drogular.

Use it when you want to understand **why** the framework is structured the way it is, which objects own request state, where application boundaries should live, and when a repeated application pattern is ready to become framework infrastructure.

## Core model

A Drogular application is built around a small set of boundaries:

```text
HTTP request
    │
    ▼
  Router
    │
    ├── GET  ──► Page ──► RenderContext ──► Components/Templates
    │
    └── POST ──► Action ──► ActionContext ──► ActionResult
                         │
                         ▼
                 Application services
                         │
                         ▼
                  Providers / domain logic
```

Pages and Actions are created per request. Long-lived application services are registered in `ApplicationServices`. Rendering state belongs to `RenderContext`; command/request input belongs to `ActionContext` or `RequestParameters`.

## Guides

- [Philosophy](philosophy.md) — the principles used when deciding what belongs in Drogular and what should remain application code.
- [Request Lifecycle](request-lifecycle.md) — request scope, Page/Action creation, rendering lifecycle, contexts, services, and error boundaries.
- [Feature Architecture](feature-architecture.md) — the feature-oriented structure validated by Portal Demo.
- [Data Provider Architecture](data-providers.md) — how Pages and Actions depend on application-facing provider interfaces instead of transports.
- [Template Rendering Pipeline](template_rendering_pipeline.md) — the shared pipeline used by `TemplatePage` and `TemplateComponent`.
- [Template Expression Engine](template_expression_engine.md) — expression values, AST, parser/evaluator boundaries, iterable expressions, lists, ranges, and operators.
- [Portal Feature Validation](portal_feature_validation_report.md) — what Projects, Users, and Departments currently prove, and which abstractions are still intentionally deferred.

## Architecture versus API Reference

Architecture documents describe ownership, boundaries, lifetimes, and design intent. They are not a substitute for the [API Reference](../reference/README.md), which remains the source for exact signatures and behavior.

Working code and tests are the final source of truth when an implementation detail and a document disagree.
