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
- 🚀 Built-in project generator and developer CLI

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
- Built-in Diagnostics with aggregate runtime metrics
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

## What's New in 0.23

Drogular 0.23 turns the server-driven UI experiments from System Monitor and PortalDemo into reusable framework capabilities.

- ⚡ Drogular Interactions for declarative fragment GET/POST flows, refresh/navigation hooks, history, retry, and offline-aware behavior
- 🎨 Drogular UI as an optional shared foundation for application shells, navigation, forms, tables, cards, status, and responsive layouts
- 🧩 `ActionRenderer` and shared request context for consistent Action → Component rendering and true request-scoped services
- 📴 Offline Read Models with cached server-rendered representations, offline navigation, locale-aware identity, and read-only mutation protection
- 🛠 Aggregate runtime diagnostics for requests, actions, rendering, Interactions, and scoped service resolution, exposed through Application Inspection and Developer Tools
- 🏗 PortalDemo migration of Projects, Departments, Roles, Project Types, and offline read flows to the new APIs
- 📊 System Monitor migration to the framework-level Interactions and UI APIs
- 🧠 Template Engine cleanup with semantic AST compilation, integer expression values, parser/runtime decomposition, and custom-function diagnostics

See [RELEASE_NOTES_0.23.md](RELEASE_NOTES_0.23.md) for the complete release notes.

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

### 0.24 — Localization & Application Experience

Drogular 0.24 will build on the 0.23 server-driven UI foundations with stronger localization tooling, a more application-oriented PortalDemo shell, and a second reference application.

**Localization**

- Load translations from external JSON files through `FileTranslationProvider`
- Use deterministic requested-locale → default-locale → key fallback behavior
- Validate catalogs with the standalone `drogular-l10n-check` utility
- Report missing and extra keys relative to a reference locale
- Enforce complete example catalogs through CTest/CI

**PortalDemo application experience**

- Move page content into a dedicated application frame
- Keep the application header persistent at the top
- Keep primary navigation in a persistent left sidebar
- Make the content area independently replaceable through Drogular Interactions
- Preserve responsive behavior for smaller screens

**Dashboard tasks**

- Add current tasks to the PortalDemo dashboard
- Model task status and priority rather than using presentation-only demo data
- Add summary views and Interaction-based task updates

**Mini Store example**

- Add a customer-facing mini store as a second reference application
- Product catalog, categories, search, filters, pagination, and product details
- Shopping cart state without introducing payment or order-processing scope
- Use external translation files as an acceptance test for the new localization API
- Exercise Drogular UI and Interactions in a public, non-admin application

### 0.25 — Client & Offline Localization

Extend the server-side localization model introduced in 0.24 to client and offline experiences without creating a second 
source of translation truth.

- Keep the server-side translation catalog as the source of truth
- Export client-consumable locale resources from the server catalog
- Add a localization manifest with the default locale and available locale resources
- Cache locale resources through the Service Worker for offline use
- Support localized Offline navigation using cached translation catalogs
- Allow language switching while offline when the requested locale is already cached
- Preserve requested-locale → default-locale → key fallback semantics on the client
- Add version/hash-based invalidation so localization resources can be refreshed independently of the application shell
- Reuse the same translation data across server rendering, online Interactions, and Offline UI

### 1.0

- Stable Public API
- Production Ready
- Long-term API Compatibility

## Design Philosophy

Drogular is designed around one simple idea:

> **Every subsystem should feel like a natural part of the framework.**

Components, dependency injection, state management, GraphQL integration, localization and Developer Tools all follow the same architectural principles, creating a consistent programming model across the entire application.

The goal of Drogular is not simply to provide more features, but to provide a cohesive framework where those features naturally work together.