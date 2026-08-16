# Project Structure

This guide explains how to grow the small project from the previous tutorial without turning `main.cpp` into the entire application.

There is no mandatory directory layout in Drogular. The structure below is a recommended starting point for applications large enough to have multiple features.

---

# Start Small

The first application used this layout:

```text
src/
├── main.cpp
├── home_service.hpp
├── home_page.hpp
└── components/
    └── home_message_component.hpp
```

Do not create directories only because the framework supports a feature. Add structure when the application actually needs it.

---

# Recommended Growing Structure

A larger application may evolve toward:

```text
src/
├── main.cpp
├── app/
│   ├── configure_services.hpp
│   └── register_routes.hpp
├── pages/
├── components/
├── actions/
├── services/
├── models/
├── stores/
└── graphql/
templates/
├── layouts/
├── components/
└── ...
public/
└── ...
```

The exact names are application choices, not framework requirements.

---

# `main.cpp`

`main.cpp` is the process entry point and should describe startup at a high level.

For example:

```cpp
#include "app/configure_services.hpp"
#include "app/register_routes.hpp"

#include <drogular/app.hpp>

int main()
{
    drogular::App app;

    app.templateRoot("templates");
    app.staticFiles("/assets", "public");

    configureServices(app);
    registerRoutes(app);

    app.run(8080);

    return 0;
}
```

A useful rule is that `main.cpp` should answer **how the application starts**, not **how a feature works**.

Keep here:

- creation of `drogular::App`
- top-level runtime profile and paths
- calls that register services, components, pages, and actions
- the listening port
- `app.run(...)`

Move feature-specific business logic, query construction, validation, and rendering elsewhere.

---

# `app/`

The optional `app/` directory is useful when startup registration becomes too large for `main.cpp`.

Typical responsibilities:

- registering application services
- registering components
- registering pages and actions
- application-wide configuration helpers

For example, service registration can be grouped in one function:

```cpp
#pragma once

#include "../services/user_service.hpp"

#include <drogular/app.hpp>

inline void configureServices(drogular::App& app)
{
    app.services().add<UserService>(
        drogular::ServiceLifetime::Singleton
    );
}
```

This is ordinary application code; Drogular does not require a special startup class.

---

# `pages/`

Pages are GET-oriented HTTP entry points that render full responses.

They should coordinate request-specific work and presentation rather than own business logic.

Examples:

```text
pages/
├── home_page.hpp
├── login_page.hpp
└── dashboard_page.hpp
```

---

# `actions/`

Actions are POST-oriented HTTP entry points for user intent and mutations.

Examples:

```text
actions/
├── login_action.hpp
├── create_todo_action.hpp
└── delete_todo_action.hpp
```

Pages and Actions are both registered during startup:

```cpp
app.page<HomePage>("/");
app.action<LoginAction>("/login");
```

---

# `components/`

Components render reusable pieces of UI and are normally registered once during startup:

```cpp
app.component<UserCardComponent>();
```

Template components can then be used by tag from templates.

---

# `services/`

Services contain application or domain behavior and integrations.

Typical responsibilities include:

- database or repository access
- GraphQL communication
- authentication backends
- domain rules
- coordination with external systems

Register shared services in `ApplicationServices` and resolve them through `RenderContext` or `ActionContext`.

---

# `models/`

Models represent application data such as `User`, `Todo`, `Product`, or `Order`.

Keep presentation-specific state out of domain models unless it is intentionally part of that model.

---

# `stores/`

Stores can coordinate shared application state where that pattern is appropriate.

Remember that application-wide mutable state must be safe for concurrent requests. A Singleton lifetime does not make the object itself thread-safe.

---

# `graphql/`

Keep application GraphQL documents, providers, serializers, and transport adapters together when GraphQL is a significant part of a feature.

For larger applications, Drogular's own `PortalDemo` goes further and groups these files **by feature** rather than keeping one global GraphQL directory. Either structure is valid; prefer the one that keeps related code easiest to find.

---

# `templates/` and `public/`

`templates/` contains server-rendered templates configured through `app.templateRoot(...)`.

`public/` commonly contains static assets exposed through `app.staticFiles(...)`.

These paths are application choices. They become meaningful only when you configure them during startup.

---

# Flat vs Feature-Oriented Layout

A flat technical layout is easy to learn:

```text
pages/
components/
services/
actions/
```

As the application grows, a feature-oriented layout may become easier to maintain:

```text
features/
├── users/
│   ├── pages/
│   ├── actions/
│   └── services/
└── projects/
    ├── pages/
    ├── actions/
    └── services/
```

`PortalDemo` is the reference for this larger feature-oriented style.

---

# Design Principles

- Keep `main.cpp` readable as a startup outline.
- Register infrastructure during startup, not during request handling.
- Keep Pages and Actions thin.
- Keep rendering in Components and templates.
- Keep application/domain behavior in Services and providers.
- Start with the smallest structure that is clear, then introduce feature modules as the codebase grows.

---

# What's Next?

Continue with [Components](components.md) to look more closely at component lifecycle, service access, and composition.
