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
- ⚡ Server-driven UI
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
| Developer Tools | Extending the Developer Tools platform |
| PortalDemo | Complete reference application architecture |

---

## Quick Start

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

The [Getting Started](docs/getting-started/README.md) guide adds the required CMake setup, templates, Components, services, dependency injection, and project organization.

### Create a new project

The Drogular CLI can create a ready-to-build application skeleton:

```bash
drogular new hello_drogular
cd hello_drogular
cmake -S . -B build
cmake --build build
./build/hello_drogular
```

The generated project includes a Page, a reusable Component, templates, a public-assets directory, and the recommended startup structure.

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

## Documentation

- 📖 [Getting Started](docs/getting-started/README.md)
- 🍳 [Cookbook](docs/cookbook/README.md)
- 🏗 [Architecture](docs/architecture/README.md)
- 📚 [API Reference](docs/reference/README.md)
- 🧩 [Examples](examples/)
- 🚀 [PortalDemo Reference Application](examples/portal_demo/)

---

## Roadmap

### 0.21

- Developer Tools Platform
- Application Inspection
- Extensible Diagnostics
- Drogular CLI and project generator
- Reference Application improvements

### 1.0

- Stable Public API
- Production Ready
- Long-term API Compatibility

### CLI Roadmap

The first Drogular CLI version intentionally stays small and focused on project creation. Future commands will build on the same external template infrastructure.

- **Next:** `drogular generate page`, `drogular generate component`
- **Later:** `drogular generate action`, `drogular generate service`, `drogular generate middleware`
- **Templates:** `drogular templates`, `drogular new --template=...`
- **After API stabilization:** GraphQL, CRUD, and feature generators

---

## Design Philosophy

Drogular is designed around one simple idea:

> **Every subsystem should feel like a natural part of the framework.**

Components, dependency injection, state management, GraphQL integration, localization and Developer Tools all follow the same architectural principles, creating a consistent programming model across the entire application.

The goal of Drogular is not simply to provide more features, but to provide a cohesive framework where those features naturally work together.