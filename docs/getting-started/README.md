# Getting Started

Welcome to Drogular.

This guide is designed to be read in order. It starts with a buildable CMake project and gradually introduces the core runtime model without assuming hidden project setup.

Complete examples show the `#include` directives and files they require so you can distinguish Drogular API requirements from application-specific code.

---

# Learning Path

## 1. [Installation](installation.md)

Create and verify a minimal CMake application.

You'll learn:

- toolchain requirements
- FetchContent integration
- the `Drogular::drogular` target
- the minimum `main.cpp`
- how to diagnose a missing Drogon package

---

## 2. [Creating a Project with the Drogular CLI](creating-a-project.md)

Generate a ready-to-build application skeleton with `drogular new`.

You'll learn:

- what files the CLI creates
- how to build and run the generated application
- how generated projects are pinned to the matching Drogular release
- how to override the Git ref when developing against another branch or commit

---

## 3. [Your First Drogular Application](your-first-drogular-application.md)

Build a complete page from real framework APIs.

You'll create:

- a Service
- a template Component
- a TemplatePage
- template files
- service/component/route registrations

The guide also explains the startup order inside `main.cpp` and shows every required include for the first complete example.

---

## 4. [Project Structure](project-structure.md)

Grow the first project without turning `main.cpp` into the entire application.

You'll see:

- what belongs in `main.cpp`
- when to introduce startup helpers
- flat technical vs feature-oriented layouts
- where Pages, Actions, Components, Services, templates, and static files belong

---

## 5. [Components](components.md)

Learn the server-side UI model.

Topics include:

- `TemplateComponent`
- component registration and tags
- lifecycle
- inputs
- service resolution through `RenderContext`
- local vs shared mutable state

---

## 6. [Dependency Injection](dependency-injection.md)

Learn how Drogular creates and scopes application services.

Topics include:

- service registration
- context-based resolution
- service factory constructor injection
- Singleton, LazySingleton, Scoped, and Transient lifetimes
- dependency graph validation

---

## 7. [Routing](routing.md)

Learn how HTTP requests reach Pages and Actions.

Topics include:

- `app.page<T>()`
- `app.action<T>()`
- route parameters
- request-scoped handler instances
- route organization
- validation/error boundaries for Actions

---

# What You'll Know Afterwards

By the end of Getting Started, you should be able to answer four practical questions:

1. **What files do I create?**
2. **What do I include?**
3. **What belongs in `main.cpp`?**
4. **How does a request reach my application code?**

From there, use:

- the [Cookbook](../cookbook/README.md) for practical patterns
- [Architecture](../architecture/README.md) for design and runtime details
- the [API Reference](../reference/README.md) for exact public interfaces
- the repository examples for complete working applications
