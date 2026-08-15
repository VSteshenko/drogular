# Drogular Philosophy

Drogular is designed as a thin application framework on top of Drogon. It adds a coherent model for pages, components, actions, rendering, dependency injection, GraphQL integration, PWA support, and developer tooling without trying to replace Drogon or hide normal C++.

## Server-First Architecture

The primary application flow is server-side:

```text
request -> Page or Action -> application services -> response
```

Pages render HTML on the server. Actions process commands and return explicit `ActionResult` values. JavaScript and PWA capabilities progressively enhance that server-first model rather than becoming a second application architecture.

## Request-Scoped UI Objects

`Page` and `ActionHandler` instances are created for each matching request. Request-local mutable state may therefore live on those objects, but shared services, globals, statics, repositories, caches, and external clients retain their own concurrency requirements.

Rendering data should normally live in `RenderContext`, especially when it must be visible to templates, child components, localization, GraphQL result handling, or scoped services.

## Components as Building Blocks

Components are reusable rendering units, not application service containers. Their lifecycle is deliberately small:

```text
onInit -> render -> onDestroy
```

The same lifecycle runner is used by production rendering and test helpers. New lifecycle hooks should only be added when repeated application needs justify them.

## Explicit State Management

Drogular does not hide mutable application state behind a global framework store. State ownership must be explicit.

- request rendering state belongs to `RenderContext`;
- action input belongs to `ActionContext`;
- session state belongs to `Session`/`SessionStore`;
- shared domain state belongs to application services, repositories, providers, databases, or explicitly synchronized stores.

A singleton lifetime does not make a mutable object thread-safe.

## Dependency Injection at Application Boundaries

`ApplicationServices` owns application service registrations and factories. Pages, Actions, and Components are default-constructed by the framework and resolve services through their request/render context.

Constructor injection is supported for **services** through service factories such as `inject()`. It is not currently automatic constructor injection for Page, Action, or Component types.

Service lifetimes describe container behavior:

- `Singleton` and `LazySingleton` are application-shared;
- `Transient` creates a new service for each container resolution;
- `Scoped` stores a factory in `ApplicationServices` and is instantiated/cached by a scope owner. In the current rendering pipeline, mutable `RenderContext::service<T>()` owns that cache.

`Scoped` should therefore be understood as a render-context scope today, not as a universal HTTP-request scope across Pages, Actions, and every child context.

## Application Boundaries Before Transport Details

Pages and Actions should depend on application-facing interfaces such as providers or repositories. GraphQL, an in-memory dataset, HTTP transport, or a database implementation belongs behind that boundary.

This allows the same Page or Action to run against production infrastructure and deterministic test infrastructure without changing application logic.

## Authentication Is Policy-Light

Drogular provides session primitives and small authentication helpers, but it does not prescribe a user model, password storage, identity provider, or authorization domain.

Applications own identity and permission rules. Framework helpers only enforce generic session-based conditions and keep Page and Action behavior consistent.

## Predictable Error Boundaries

Expected validation failures should be explicit. In the Action pipeline, `ActionValidationError` maps to `400 Bad Request`. Unexpected exceptions are logged and translated to a safe `500` response without exposing internal exception messages.

Framework-specific operational failures derive from `DrogularError`, while application code remains free to define its own domain errors.

## Progressive Enhancement

PWA and browser-side behavior should enhance a working server-rendered application. Offline pages, manifests, service workers, and client-side helpers should not force Pages, Actions, providers, or domain logic into a browser-specific architecture.

## GraphQL Is a Transport, Not the Domain Model

Drogular provides GraphQL clients, document builders, server support, execution contexts, and result types. Application code should still place a provider or service boundary between UI code and transport-specific GraphQL operations when the feature has meaningful domain behavior.

## Simplicity Over Premature Abstraction

A repeated folder structure is not enough evidence for framework infrastructure. Drogular promotes an abstraction only after the same architectural need appears in multiple independent features and the abstraction removes real duplication without hiding domain behavior.

The preferred evolution path is:

```text
real application
    -> repeated pattern
    -> small application helper
    -> validated reusable abstraction
    -> public framework API
```

## Predictability

A small explicit API is preferred to hidden convention. Object lifetimes, context ownership, error translation, and service resolution should be understandable from the code that registers and uses them.

## Long-Term Stability

Before 1.0, API review is the time to remove accidental complexity and clarify contracts. After an API becomes stable, compatibility should outweigh cosmetic redesign. Documentation, examples, and Portal Demo are therefore treated as design tools: if an API is difficult to explain correctly, its design deserves another look.
