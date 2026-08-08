# API Reference

Reference documentation for Drogular's supported public API.

The API Reference is organized by logical subsystem rather than by header-file layout. Each section documents the types developers use directly, their public members, runtime behavior, and relationships to the rest of the framework.

Use the [Getting Started](../getting-started/README.md) guide to learn the framework fundamentals and the [Cookbook](../cookbook/README.md) to solve common application tasks.

---

# Available Sections


## Application

Configure the application, register high-level framework features, select a runtime profile, and start the server.

- [Application](application/README.md)

## Pages

Define route-mounted HTML pages, use template-backed rendering, and add common page or PWA context values.

- [Pages](pages/README.md)

## Components

Render reusable UI fragments, accept typed inputs, compose slots, and register component tags.

- [Components](components/README.md)

## Rendering

Store request-scoped render data, resolve services, access request metadata, translate text, and coordinate GraphQL result data.

- [Rendering](rendering/README.md)

## Actions

Handle POST commands, read request input and services, manage sessions, and return redirects, HTML, JSON, or files.

- [Actions](actions/README.md)

## Dependency Injection

Register application services, select lifetimes, perform constructor injection, and validate dependency metadata.

- [Dependency Injection](dependency-injection/README.md)

## Forms and Validation

Validate submitted form values, inspect field errors, and read required typed input.

- [Forms and Validation](forms-and-validation/README.md)

## Localization

Resolve request locales and translate application keys through an application-defined provider.

- [Localization](localization/README.md)

## State Management

Store mutable values and notify C++ subscribers when replacement values are published.

- [State Management](state-management/README.md)

---

# Planned Sections

The following sections will be added as the reference is expanded:

- Routing
- GraphQL
- Authentication and Sessions
- Pagination and Request Parameters
- PWA
- Static Files
- Developer Tools
- Testing

Only APIs confirmed as supported public contracts will be included.
