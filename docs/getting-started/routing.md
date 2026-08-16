# Routing

Routing connects incoming HTTP requests to Drogular Pages and Actions.

For the high-level application API, the common split is:

- `app.page<T>(path)` for GET page rendering
- `app.action<T>(path)` for POST mutations and user intent

---

# Register Routes During Startup

A small application can register routes directly in `main.cpp`:

```cpp
#include "actions/create_todo_action.hpp"
#include "pages/todo_page.hpp"

#include <drogular/app.hpp>

// ...

app.page<TodoPage>("/");
app.action<CreateTodoAction>("/todos/create");
```

A larger application can move these calls into a `registerRoutes(app)` helper while keeping the same API.

---

# Pages

Pages are full renderable responses registered for GET requests:

```cpp
app.page<HomePage>("/");
app.page<UserPage>("/users/{id}");
```

Drogular creates a fresh Page instance for each matching HTTP request.

Pages normally prepare render data in `onInit(RenderContext&)` and render HTML through `Page` or `TemplatePage`.

---

# Actions

Actions are POST request handlers:

```cpp
app.action<LoginAction>("/login");
app.action<CreateTodoAction>("/todos/create");
```

Drogular also creates a fresh Action instance for each request. Actions receive an `ActionContext`, read form/request data, resolve services, and return an `ActionResult`.

Use Pages for presentation and Actions for mutations rather than putting POST handling into a UI component.

---

# Route Parameters

Routes may contain parameters:

```cpp
app.page<UserPage>("/users/{id}");
```

Pages read route parameters from `RenderContext`; Actions use `ActionContext`.

Required values should use the corresponding `requireRouteParam()` API so a missing route parameter is treated as an explicit context error instead of silently becoming an empty value.

---

# Request Flow

A rendered page follows this high-level path:

```text
GET request
    ↓
Router
    ↓
fresh Page
    ↓
RenderContext
    ↓
Components / Services
    ↓
HTML response
```

A mutation follows:

```text
POST request
    ↓
Router
    ↓
fresh Action
    ↓
ActionContext
    ↓
Validation / Services
    ↓
ActionResult
```

`ActionValidationError` is translated by the Router into `400 Bad Request`; unexpected action exceptions become a safe `500 Internal Server Error` response.

---

# Keep HTTP Entry Points Thin

Pages and Actions should coordinate request work rather than own the application's business rules.

A common boundary is:

```text
Page / Action
      ↓
application Service or Provider
      ↓
repository / GraphQL / external system
```

This keeps routing code small and makes application behavior reusable outside one HTTP endpoint.

---

# Route Organization

Keep route registration easy to discover.

For a small application, `main.cpp` is enough. For a larger application, group registrations in one startup helper or by feature:

```cpp
void registerUserRoutes(drogular::App& app)
{
    app.page<UsersPage>("/users");
    app.page<UserEditPage>("/users/{id}/edit");
    app.action<UpdateUserAction>("/users/{id}/update");
}
```

The helper is ordinary application code; the framework does not require a special routing module type.

---

# What's Next?

You now have the core Getting Started model: build configuration, startup organization, Components, dependency injection, Pages, and Actions.

Continue with the [Cookbook](../cookbook/README.md) for task-oriented patterns, or read the [Request Lifecycle](../architecture/request-lifecycle.md) when you want a deeper architectural view of the same request path.
