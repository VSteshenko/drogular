<p align="center">
    <img src="assets/logo.png" width="200" alt="Drogular Logo">
</p>

# Drogular

> **Modern C++ application framework for Drogon with server-driven UI, GraphQL, PWA and extensible Developer Tools.**

Build modern web applications in C++ using a unified architecture for components, dependency injection, state management, GraphQL and Developer Tools.

**A cohesive application framework — not a collection of unrelated libraries.**

---

## Why Drogular?

Building modern web applications in C++ often requires combining independent libraries for routing, templates, dependency injection, state management, GraphQL clients and development tooling.

Drogular takes a different approach.

It provides these capabilities as parts of a single application framework with a consistent programming model. Pages, components, services, actions, state management, GraphQL integration and Developer Tools are designed to work together instead of being assembled from unrelated libraries.

---

## Core Principles

- 🧩 Component-based architecture
- ⚡ Server-rendered UI with progressive enhancement where needed
- 💉 Dependency Injection everywhere
- 🔄 Reactive State Management
- 🌐 Native GraphQL integration
- 🌍 Built-in Localization
- 📱 Progressive Web Apps
- 🛠 Developer Tools built on public APIs
- 🔌 Extensible architecture

---

## Architecture

```text
Application
    ├── Pages
    ├── Components
    ├── Services
    ├── Actions
    ├── State
    ├── GraphQL
    ├── Localization
    └── Developer Tools
```

Every subsystem follows the same architectural principles, creating a consistent programming model throughout the application.

---

## Developer Tools

Developer Tools are a first-class part of Drogular rather than an external utility.

```text
             Application

                   │
                   ▼
       Application Inspection

                   │
                   ▼
          Inspection JSON API

        ┌──────────┴──────────┐
        ▼                     ▼
 Diagnostics Page      External Tools
```

Developer Tools include:

- Development Profile
- Built-in Diagnostics
- Application Inspection
- Extensible Inspection Contributors
- Custom Developer Components
- Public JSON contract for external tools
- IDE-ready architecture

Applications and libraries can contribute their own inspection sections and custom visualizations without modifying Drogular itself.

---

## Reference Application

**PortalDemo** is the reference application for Drogular.

It demonstrates the recommended architecture, including:

- Authentication
- Dependency Injection
- Components
- Reactive State Management
- GraphQL
- Localization
- Developer Tools

Rather than being a collection of isolated examples, PortalDemo shows how these features work together in a complete application.

---

## Examples

| Example | Demonstrates |
|----------|--------------|
| TodoPWA | Components, State Management, Forms & Validation |
| Auth Sample | Authentication & Sessions |
| Developer Tools | Extending the Developer Tools platform |
| Repository Sample | Repository pattern and data access |
| System Monitor PWA | Live server-rendered fragments, PWA behavior, and remote/system hardware monitoring |
| PortalDemo | Complete reference application architecture |

---

## Quick Start

The fastest way to start a Drogular application is with the CLI:

```bash
drogular new hello_drogular
cd hello_drogular
cmake -S . -B build
cmake --build build
./build/hello_drogular
```

The generated project is pinned to the matching Drogular release and includes a Page, reusable Component, templates, 
public assets, and the recommended startup structure.

The CLI also supports explicit output paths and multiple embedded starters:

```bash
drogular templates
drogular new examples/admin --template minimal
drogular new apps/MyPWA --template pwa
```

`minimal` is the smallest recommended Drogular application. `pwa` is a ready-to-run installable starter with a manifest
, service worker, offline fallback, responsive UI, and short `Tip:` comments that explain the important extension points.

A complete minimal route needs only a Page and `drogular::App`:

```cpp
#include <drogular/app.hpp>
#include <drogular/page.hpp>
#include <drogular/render_context.hpp>

#include <string>

class HomePage final : public drogular::Page
{
public:
    std::string render(drogular::RenderContext&) override
    {
        return "<h1>Hello Drogular</h1>";
    }
};

int main()
{
    drogular::App app;

    app.page<HomePage>("/");
    app.run(8080);

    return 0;
}
```

The [Getting Started](docs/getting-started/README.md) guide adds the required CMake setup, templates, Components, services, dependency injection, CLI workflow, and project organization.

---

## Feature Overview

| Feature | Status |
|----------|:------:|
| Components | ✅ |
| Routing | ✅ |
| Dependency Injection | ✅ |
| State Management | ✅ |
| Forms & Validation | ✅ |
| GraphQL Integration | ✅ |
| Localization | ✅ |
| Progressive Web Apps | ✅ |
| Developer Tools | ✅ |

---

## What's New in 0.21

Drogular 0.21 focuses on developer experience, diagnostics, and production readiness.

- 🚀 Standalone Drogular CLI with reproducible project generation
- 🔍 Application Inspection API
- 🛠 Extensible Developer Tools platform
- 📊 Built-in Diagnostics Page and JSON inspection endpoint
- ⚙️ Request-scoped dependency injection across request contexts
- 🔒 Thread-safety improvements for template caching and lazy singletons
- 📚 Major expansion of Getting Started, Cookbook, and API Reference documentation

See [RELEASE_NOTES_0.21.md](RELEASE_NOTES_0.21.md) for the complete release notes.

---

## Documentation

- 📖 [Getting Started](docs/getting-started/README.md)
- 🍳 [Cookbook](docs/cookbook/README.md)
- 🏗 [Architecture](docs/architecture/README.md)
- 📚 [API Reference](docs/reference/README.md)
- 🧩 [Examples](examples/)
- 🚀 [PortalDemo Reference Application](examples/portal_demo/)

---

## Roadmap

### 0.22 — Developer Productivity

Current progress: the standalone CLI, embedded project-template registry, `minimal` starter, and `pwa` starter are
already implemented on `main`. Feature/CRUD scaffolding and the remaining productivity work are still roadmap items.

- CLI Generators
- Feature-first scaffolding
- Project Templates
- Feature / CRUD generation
- Asset Pipeline
- Localization 2.0
- PortalDemo expansion
- Architecture, Best Practices, and CLI documentation

### 0.23 — Server-driven UI & Framework Foundations

Drogular 0.23 is planned to turn the successful System Monitor experiments into reusable framework capabilities while keeping JavaScript and CSS optional, small, and composable.

**Drogular Interactions**

- Declarative fragment requests with `dg-get`, `dg-target`, and `dg-trigger`
- Load, polling, change, and debounced input triggers
- Standard loading, ready, empty, and error states
- Polling groups for coordinated live UI
- Failure limits with pause and explicit resume
- First-class Component / Fragment rendering from Actions
- Finish separating the interaction runtime from application-specific connection/status markup

**Drogular UI**

A small optional UI foundation rather than a full CSS framework:

- `dg-button`
- `dg-card`
- `dg-toolbar`
- `dg-status`
- `dg-badge`
- `dg-segmented`
- Semantic variants: `neutral`, `info`, `success`, `warning`, and `danger`

The System Monitor experiment also validated a server-driven presentation model: C++ Components choose semantic state and UI variants, templates compose the primitives, and Drogular UI owns the shared visual treatment. Application-specific layout, branding, and domain presentation remain application-owned.

Drogular UI and Drogular Interactions are intentionally independent: applications can use either one alone or combine them. The goal is to remove repeated presentation boilerplate without turning Drogular into a general-purpose CSS framework.

**Template Engine cleanup**

- Precompile interpolation expressions
- Compile component tags/attributes into semantic AST
- Split `template_runtime.cpp`
- Split Expression parser/lexer
- Integer `ExpressionValue`
- Runtime diagnostics for custom functions

### 1.0

- Stable Public API
- Production Ready
- Long-term API Compatibility

## Design Philosophy

Drogular is designed around one simple idea:

> **Every subsystem should feel like a natural part of the framework.**

Components, dependency injection, state management, GraphQL integration, localization and Developer Tools all follow the same architectural principles, creating a consistent programming model across the entire application.

The goal of Drogular is not simply to provide more features, but to provide a cohesive framework where those features naturally work together.