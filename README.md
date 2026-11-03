<p align="center">
    <img src="assets/logo.png" width="200" alt="Drogular Logo">
</p>

# Drogular

**Language:** **English** | [Deutsch](README.de.md)

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
| Mini Store | Localized storefront and staff workflows, cart/checkout, order lifecycle, Interactions, and PWA behavior |

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

## What's New in 0.24

Drogular 0.24 expands the server-driven application model with file-backed localization, translation validation, a persistent PortalDemo application shell, and the new Mini Store reference application.

- 🌐 `FileTranslationProvider` for external JSON translation catalogs with deterministic requested-locale → default-locale → key fallback
- ✅ `drogular-l10n-check` plus CTest/CI catalog validation for missing and extra translation keys
- 🏗 PortalDemo application frame with persistent header/sidebar navigation and an independently replaceable content area
- 📋 PortalDemo dashboard presentation for current project work, while richer task modeling remains planned for 0.25
- 🛍 Mini Store reference application with localized Storefront and Staff surfaces, catalog, cart, checkout, stock validation, product/category management, and order workflow
- ⚡ Mini Store coverage of Drogular UI, Interactions, canonical/history navigation, themes, and PWA/offline behavior
- 🧪 Expanded application-level regression coverage for the Mini Store flows and localization contracts

See [RELEASE_NOTES_0.24.md](RELEASE_NOTES_0.24.md) for the complete release notes.

---

## Documentation

- 📖 [Getting Started](docs/getting-started/README.md)
- 🍳 [Cookbook](docs/cookbook/README.md)
- 🏗 [Architecture](docs/architecture/README.md)
- 📚 [API Reference](docs/reference/README.md)
- 🧩 [Examples](examples/)
- 🚀 [PortalDemo Reference Application](examples/portal_demo/)
- 🛍 [Mini Store Reference Application](examples/mini_store/)

---

## Roadmap

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
- Extend the existing `NavigationContext` return-target validation and propagation helpers with context-level navigation integration where useful
- Extend PortalDemo task modeling with first-class task status and priority instead of presentation-only current-work data
- Add task summary views and Interaction-based task updates

### 0.26 — Background Jobs & Progress

Introduce a first-class model for long-running and background work so applications such as Atlasys can execute, observe, and control operations without tying their lifetime to a single HTTP request.

- Add a framework-level Job model with identity, lifecycle state, result/error state, and execution metadata
- Add Job progress reporting for long-running operations
- Add cooperative Job cancellation with explicit cancellation state and semantics
- Add an SSE/event stream for live Job state and progress updates
- Add `JobsDemo` as a reference application for starting, observing, cancelling, and reconnecting to Jobs
- Add Job diagnostics for lifecycle transitions, execution time, failures, cancellation, and active/completed Job inspection

### 0.27 — Health, Readiness & Authentication Context

Add the operational and identity foundations required by applications such as Atlasys when running as production services.

- Add first-class health and readiness endpoints with extensible application checks
- Extend the authentication model with a richer Principal/authentication context for application and service authorization
- Extend `AuthSample` to demonstrate Principal data, authentication state, authorization-oriented usage, and the new auth extension points

### 0.28 — Audit & Metrics

Add framework-level observability extension points so applications can integrate Drogular activity with operational, compliance, and monitoring infrastructure.

- Add Audit hooks for security-sensitive and application-defined operations without coupling the framework to a specific audit backend
- Add a Metrics exporter abstraction for exposing framework and application metrics to external monitoring systems

### 0.29 — Windows Platform Support & Cross-Platform Hardening

Extend the supported platform matrix without changing the planned scope of 0.25–0.28.

- Support Windows x64 builds with MSVC and Visual Studio
- Add Windows CI with the complete test suite
- Audit and harden portability across Linux, macOS, and Windows
- Keep MinGW outside the initial 0.29 scope

### 1.0

- Stable Public API
- Production Ready
- Long-term API Compatibility

## Design Philosophy

Drogular is designed around one simple idea:

> **Every subsystem should feel like a natural part of the framework.**

Components, dependency injection, state management, GraphQL integration, localization and Developer Tools all follow the same architectural principles, creating a consistent programming model across the entire application.

The goal of Drogular is not simply to provide more features, but to provide a cohesive framework where those features naturally work together.