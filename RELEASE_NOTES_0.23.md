# Drogular 0.23.0

Drogular 0.23 turns the server-driven UI work proven in System Monitor and PortalDemo into reusable framework capabilities. 
The release introduces Drogular Interactions and Drogular UI, strengthens the Action/rendering and request-scope model, 
adds read-only offline representations, and expands Developer Tools with aggregate runtime diagnostics.

## Highlights

### Drogular Interactions

Drogular Interactions provides a small declarative runtime for progressively enhanced server-driven interfaces.

The 0.23 API includes:

- fragment GET requests with `dg-get`;
- form and mutation POST requests with `dg-post`;
- explicit fragment targets with `dg-target`;
- load, polling, change, and debounced-input triggers;
- loading, ready, empty, and error states;
- coordinated polling groups;
- bounded failure handling with pause and explicit resume;
- URL/history replacement for browser-style filtering;
- `dg-reset` and reset values for filter forms;
- success refresh hooks for dependent fragments;
- success navigation hooks for terminal mutation flows.

The runtime is served by Drogular through `/__drogular/assets/interactions.js` when enabled with `app.interactions()`.

Interaction requests identify themselves to the server, and `ActionContext::isInteraction()` exposes that protocol 
detail through the public Action API. Server redirects remain normal HTTP redirects and are handled before fragment 
insertion, keeping authentication redirects separate from application-level success navigation.

### Drogular UI

0.23 introduces Drogular UI as an optional presentation foundation rather than a general-purpose CSS framework.

It provides shared primitives for buttons, cards, toolbars, status indicators, badges, segmented controls, application
shells, navigation, forms, tables, details, pagination, empty states, and collapsible sections. Semantic variants include 
neutral, info, success, warning, and danger states.

The framework assets are served through `/__drogular/assets/ui.css` and `/__drogular/assets/ui.js` when enabled with `app.ui()`.

Drogular UI and Drogular Interactions remain independent. Applications can use either subsystem alone or combine them.

## Actions, Rendering, and Request Context

### ActionRenderer

`ActionRenderer` provides the framework-level bridge from an `ActionContext` to Component rendering.

It preserves the request context while allowing an Action to configure a `RenderContext`, render a Component tree, and 
return an `ActionResult` with an explicit HTTP status. Component destruction remains exception-safe through the normal 
component renderer lifecycle.

### Shared RequestContextState

Action and rendering execution now share an internal request state containing:

- the Drogon HTTP request;
- application services;
- route parameters;
- the request `ServiceScope`;
- runtime diagnostics wiring.

This fixes the previous architectural gap where Action and Render contexts could create independent scoped-service caches.
A scoped service now has true request lifetime across Action → RenderContext transitions and child RenderContexts.

## Offline Read Models

Drogular Interactions gains an opt-in read-only offline representation layer through `app.offlineReadModels()`.

The implementation includes:

- exact cached GET representations backed by IndexedDB;
- representation identity that includes request, locale/context, and scope;
- cached application shells and same-origin offline navigation;
- independent connection, data, and interaction-capability state;
- explicit read-only behavior that blocks mutations instead of queueing writes;
- cache-only locale switching with reconciliation after reconnect.

PortalDemo validates the model across Projects, Departments, and Users, including navigation between cached areas. 
Offline Read Models deliberately preserve server-rendered HTML instead of introducing a second client-side domain model.

## PortalDemo

PortalDemo was migrated to the 0.23 APIs and now acts as the primary reference for larger server-driven business applications.

The release includes Interaction-based flows for:

- Projects browser, filtering, sorting, pagination, create, edit, and delete;
- Departments browser and CRUD flows;
- Roles create, edit, list, and delete flows;
- Project Types create, edit, list, and delete flows;
- persistent filters and return navigation where application state requires them;
- read-only offline Projects, Departments, and Users.

These migrations established three reusable mutation compositions: local target replacement, local replacement plus 
dependent refresh, and terminal success navigation.

## System Monitor PWA

System Monitor now consumes the framework-level Drogular Interactions and Drogular UI APIs instead of carrying parallel 
application-specific infrastructure.

The migration covers live dashboard updates, reconnect/retry behavior, offline state, hardware fragments, and shared 
presentation primitives while preserving application-specific monitoring and hardware UI where it belongs.

## Runtime Diagnostics and Developer Tools

0.23 adds thread-safe aggregate `RuntimeDiagnostics` for runtime execution without retaining request history or introducing profiler-style storage.

The collected metrics cover:

- requests, Actions, rendered Actions, and redirects;
- Interaction GET/POST requests, successful responses, client errors, and redirects;
- rendered Components, render failures, and maximum component-tree depth;
- scoped-service resolutions, cache hits, and created service instances.

The runtime layer records inexpensive counters and remains independent from Developer Tools. A core diagnostics contributor 
exposes immutable snapshots through the existing extensible Application Inspection sections contract.

Developer Tools includes built-in renderers for Runtime, Interactions, and Service Scopes. The Application Inspection schema
remains version 3 because the new data uses the existing extensible sections mechanism.

## Template Engine Cleanup

The 0.23 template work continues the move from ad-hoc runtime interpretation toward a clearer compiled semantic model.

Changes include:

- precompiled interpolation expressions;
- semantic AST compilation for component tags and attributes;
- integer `ExpressionValue` support;
- parser/lexer decomposition;
- runtime decomposition of the template engine;
- diagnostics for custom functions.

## Documentation and Testing

Documentation was synchronized with the new Action/rendering ownership model, Runtime Diagnostics, Application Inspection 
sections, and Developer Tools presentation.

Regression coverage was added throughout the 0.23 work for Interaction contracts, CRUD flows, request-scoped services, 
ActionRenderer, component rendering diagnostics, offline read behavior, and diagnostics contributors/renderers.

## Looking Ahead

0.24 will focus on Localization & Application Experience: external translation sources and validation tooling, 
a persistent PortalDemo application frame, dashboard task workflows, and a new Mini Store reference example.
