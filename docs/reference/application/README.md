# Application

The Application subsystem provides the top-level API for configuring and starting a Drogular application.

`App` owns application-wide options, the dependency injection container, routing registration, runtime profile selection, and Developer Tools activation.

---

# Types

## [`App`](app.md)

Main application entry point used to configure routes, components, services, static files, PWA behavior, runtime profiles, and Developer Tools before starting the server.

## [`ApplicationOptions`](application-options.md)

Stores application-wide configuration for templates, static files, cache behavior, and the service worker path.

## [`ApplicationProfile`](application-profile.md)

Selects the application runtime profile: Development, Testing, or Production.

---

# Typical Flow

```text
Create App
    │
    ▼
Configure options and services
    │
    ▼
Register components, pages and actions
    │
    ▼
Select runtime profile
    │
    ▼
Run the HTTP server
```

```cpp
drogular::App app;

app.templateRoot("templates")
    .profile(drogular::ApplicationProfile::Development)
    .page<HomePage>("/")
    .action<LoginAction>("/login");

app.services().add<AuthService>(
    drogular::ServiceLifetime::Singleton
);

app.run(8080);
```

---

# Getting Started

- [Your First Drogular Application](../../getting-started/your-first-drogular-application.md)
- [Routing](../../getting-started/routing.md)
- [Dependency Injection](../../getting-started/dependency-injection.md)
