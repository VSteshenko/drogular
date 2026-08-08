# `App`

**Namespace:** `drogular`  
**Header:** `<drogular/app.hpp>`  
**Kind:** Class

## Purpose

`App` is the main application entry point.

It owns application-wide options, the dependency injection container, page and action routing, component registration, PWA registration, runtime profile selection, application inspection, Developer Tools activation, and server startup.

Configuration methods return `App&` unless otherwise noted, so calls can be chained.

---

## Role in Drogular

`App` is the composition root of a Drogular application.

It connects application configuration with the major framework subsystems and exposes the registration surface used during startup.

```text
Application startup
        │
        ▼
       App
        │
        ├── ApplicationOptions
        ├── ApplicationServices
        ├── Router
        ├── Pages and Actions
        ├── Components
        ├── GraphQL
        ├── Static Files and PWA
        └── Developer Tools
```

Most application-wide framework configuration begins here. After registration is complete, `App::run()` hands execution to Drogon.

---

## Construction

```cpp
drogular::App app;
```

A new application starts with:

- `ApplicationProfile::Production`;
- Developer Tools disabled;
- inspection disabled;
- diagnostics page disabled;
- default `ApplicationOptions` values.

The constructor also connects the application's service container to its `ApplicationOptions` instance.

---

## Configuration Access

### `options()`

```cpp
ApplicationOptions& options();
const ApplicationOptions& options() const;
```

Returns the application-wide options object.

### `services()`

```cpp
ApplicationServices& services();
```

Returns the [`ApplicationServices`](../dependency-injection/application-services.md) dependency injection container used by pages, actions, components, and framework services.

```cpp
app.services().add<UserRepository>(
    drogular::ServiceLifetime::Singleton
);
```

---

## Templates

### `templateRoot()`

```cpp
App& templateRoot(
    std::filesystem::path root
);
```

Sets `ApplicationOptions::templateRoot()`.

### `templateCache()`

```cpp
App& templateCache(bool enabled);
```

Enables or disables template source caching.

---

## GraphQL

### `graphQLClient()`

```cpp
App& graphQLClient(
    std::shared_ptr<GraphQLClient> client
);
```

Registers the GraphQL client exposed to application rendering infrastructure through the service container.

---

## Route Registration

### `page<PageType>()`

```cpp
template <typename PageType>
App& page(const std::string& path);
```

Registers a page instance for a GET route. See [`Page`](../pages/page.md) and [`TemplatePage`](../pages/template-page.md).

`PageType` must inherit from `drogular::Page` and must be default-constructible because registration creates it with `std::make_shared<PageType>()`.

```cpp
app.page<HomePage>("/");
app.page<UserEditPage>("/users/{id}/edit");
```

### `action<ActionType>()`

```cpp
template <typename ActionType>
App& action(const std::string& path);
```

Registers an [`ActionHandler`](../actions/action-handler.md) instance for a POST route.

`ActionType` must inherit from [`drogular::ActionHandler`](../actions/action-handler.md) and must be default-constructible.

```cpp
app.action<LoginAction>("/login");
```

### `component<ComponentType>()`

```cpp
template <typename ComponentType>
App& component(const std::string& tag);

template <typename ComponentType>
App& component();
```

Registers a [`Component`](../components/component.md) type with the application's [`ComponentRegistry`](../components/component-registry.md).

The overload without a tag uses `ComponentType::tag`.

```cpp
app.component<TodoItemComponent>();
app.component<AlertComponent>("app-alert");
```

---

## Static Files and PWA

### `staticFiles()`

```cpp
App& staticFiles(
    std::string routePrefix,
    std::filesystem::path directory
);
```

Adds a [`StaticFileMapping`](../static-files/static-file-mapping.md) to `ApplicationOptions`.

The mapping is registered with the router when `run()` is called. See [Static Files](../static-files/README.md) for path resolution, MIME detection, and cache behavior.

### `staticFileEtag()`

```cpp
App& staticFileEtag(bool enabled);
```

Enables or disables ETag generation for static file responses.

### `staticFileLastModified()`

```cpp
App& staticFileLastModified(bool enabled);
```

Enables or disables Last-Modified headers for static file responses.

### `staticFileCacheProfile()`

```cpp
App& staticFileCacheProfile(
    StaticFileCacheProfile profile
);
```

Applies a predefined [`StaticFileCacheProfile`](../static-files/static-file-cache-profile.md) through `ApplicationOptions`.

### `staticFileCache()` and `staticFileCacheMaxAge()`

```cpp
App& staticFileCache(bool enabled);

App& staticFileCacheMaxAge(
    std::chrono::seconds maxAge
);
```

Configure whether static-file responses use browser cache storage and the `Cache-Control` max-age value.

```cpp
app.staticFileCache(true)
   .staticFileCacheMaxAge(std::chrono::hours(1));
```

See the [Static Files](../static-files/README.md) reference for the complete caching and conditional-request pipeline.

### `serviceWorker()`

```cpp
App& serviceWorker(
    std::filesystem::path path
);
```

Stores a service worker file that is registered at `/service-worker.js` during `run()`.

### `offlinePage<PageType>()`

```cpp
template <typename PageType>
App& offlinePage(
    std::string route = "/__offline"
);
```

Registers a factory for a dynamic offline page.

The route is added when `run()` is called.

`PageType` is created with `std::make_shared<PageType>()` and therefore must be default-constructible.

---

## Runtime Profile

### `profile()`

```cpp
App& profile(ApplicationProfile profile);
ApplicationProfile profile() const;
```

Sets or returns the current application profile.

Applying `Development` enables Developer Tools automatically unless `disableDeveloperTools()` was called first.

A newly constructed application uses `Production`.

Changing from Development to another profile throws `std::logic_error` when Developer Tools have already been registered.

---

## Inspection and Developer Tools

### `inspect()`

```cpp
ApplicationInspection inspect() const;
```

Builds and returns a snapshot of registered routes, static file mappings, service worker route, offline page route, components, services, diagnostics entries, and custom inspection contributions.

Calling `inspect()` does not start the server.

### `enableDeveloperTools()`

```cpp
App& enableDeveloperTools();
```

Enables the complete Developer Tools subsystem.

The current implementation enables the diagnostics page, which also enables the public inspection endpoint.

The operation is idempotent.

### `disableDeveloperTools()`

```cpp
App& disableDeveloperTools();
```

Disables automatic Developer Tools activation before registration occurs.

Calling this method after Developer Tools, inspection, or diagnostics have already been registered throws `std::logic_error`.

### `enableInspection()`

```cpp
App& enableInspection();
```

Registers the public application inspection JSON endpoint and its supporting services.

The operation is idempotent.

### `enableDiagnosticsPage()`

```cpp
App& enableDiagnosticsPage();
```

Registers the diagnostics browser application, its assets, component registry endpoint, and inspection endpoint.

The operation is idempotent and marks Developer Tools as enabled.

### `developerToolsContributor()`

```cpp
App& developerToolsContributor(
    std::shared_ptr<DeveloperToolsContributor> contributor
);
```

Adds a contributor that can append application-specific data to inspection snapshots.

### `inspectionContributor()`

```cpp
App& inspectionContributor(
    std::shared_ptr<DeveloperToolsContributor> contributor
);
```

Alias for `developerToolsContributor()`.

### `developerToolsComponent()`

```cpp
App& developerToolsComponent(
    std::string name,
    std::string module
);
```

Registers a trusted browser-side renderer module for the diagnostics application.

---

## Server Startup

### `run()`

```cpp
void run(unsigned short port);
```

Before starting Drogon, `run()` registers:

- configured static file mappings;
- the service worker route, when configured;
- the offline page route, when configured.

It then listens on `0.0.0.0:<port>` and starts the Drogon event loop.

The call blocks while the server is running.

---

## Errors and Constraints

- `page<PageType>()` requires `PageType` to inherit from `Page` and be default-constructible.
- `action<ActionType>()` requires `ActionType` to inherit from `ActionHandler` and be default-constructible.
- `offlinePage<PageType>()` requires a default-constructible page type.
- `disableDeveloperTools()` throws after Developer Tools registration has begun.
- `profile()` can throw when attempting to move away from Development after Developer Tools are registered.
- `staticFileCache()` and `staticFileCacheMaxAge()` currently have no implementation definitions.

---

## Example

```cpp
int main() {
    drogular::App app;

    app.templateRoot("examples/auth_sample/templates")
        .staticFiles(
            "/assets",
            "examples/auth_sample/public"
        )
        .profile(
            drogular::ApplicationProfile::Development
        )
        .page<HomePage>("/")
        .page<LoginPage>("/login")
        .action<LoginAction>("/login")
        .action<LogoutAction>("/logout");

    app.services().add<AuthService>(
        drogular::ServiceLifetime::Singleton
    );

    app.run(8081);
}
```

---

## Related Types

- [`PWA`](../pwa/README.md)
- [`Router`](../routing/router.md)
- [`ApplicationOptions`](application-options.md)
- [`ApplicationProfile`](application-profile.md)
- [`ApplicationServices`](../dependency-injection/application-services.md)
- `ApplicationInspection`
- [`Page`](../pages/page.md)
- [`ActionHandler`](../actions/action-handler.md)
- `GraphQLClient`

## See Also

- [Your First Drogular Application](../../getting-started/your-first-drogular-application.md)
- [Routing](../../getting-started/routing.md)
- [Dependency Injection](../../getting-started/dependency-injection.md)
- [Developer Tools example](../../../examples/developer_tools/README.md)
