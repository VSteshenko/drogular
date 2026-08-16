# Dependency Injection

Dependency Injection is the standard way to create and provide services in Drogular applications.

All official examples and documentation use dependency injection.

Although C++ allows services to be created manually, Drogular applications should register shared application services with the dependency injection container and resolve them through the current context.

---

# Why Dependency Injection?

Without dependency injection, classes become responsible for constructing the objects they need.

This increases coupling and makes applications harder to test, maintain, and extend.

With dependency injection, service creation is centralized and request-handling code resolves registered interfaces instead of constructing concrete infrastructure itself.

---

# Resolving Dependencies

Drogular's service container manages application services. Pages, Actions, and Components themselves are currently created with default constructors by their framework registries. They resolve the services they need from the current context.

For example, a Page can resolve a required service during initialization:

```cpp
void onInit(drogular::RenderContext& context) override {
    auto service = context.requireService<HomeService>();
    // use service to populate request-specific render data
}
```

Actions use `ActionContext::requireService<T>()` in the same style.

Constructor injection is available when registering **services**. The `inject<Service, Dependencies...>()` helper builds a service factory and records dependency metadata for validation. It does not currently constructor-inject Page, Action, or Component objects.

---

# Registering Services

Before a service can be injected, it must be registered with the dependency injection container.

The container manages object creation according to the configured lifetime.

---

# Service Lifetimes

Drogular currently supports four service lifetimes.

## Singleton

One instance is created and shared for the lifetime of the application.

Typical examples:

- configuration
- logging
- application-wide caches

---

## LazySingleton

Similar to a Singleton, but the instance is created only when it is first requested.

Useful when object construction is expensive or the service may never be used.

---

## Scoped

A single instance is shared within one HTTP request.

`RenderContext` and `ActionContext` each resolve scoped registrations through a request-owned service scope. Child render contexts inherit the same service scope, so a scoped service resolved by a page and any nested component rendering is the same instance for that request. A later HTTP request receives a new scope and therefore a new scoped instance.

---

## Transient

A new instance is created every time the service is requested.

Transient services are useful for lightweight, stateless objects.

---

# Choosing a Lifetime

Choose the lifetime that matches the responsibility of the service.

| Lifetime | Typical Use |
|----------|-------------|
| Singleton | Shared application services |
| LazySingleton | Expensive shared services |
| Scoped | Per-request services |
| Transient | Lightweight stateless services |

Most services naturally fit into one of these four categories.

---

# Best Practices

- Keep service dependencies explicit.
- Register services during application startup.
- Choose the narrowest appropriate lifetime.
- Avoid global objects and service locators.

---

# Avoid Manual Construction

Avoid constructing shared application services directly inside Components, Pages, or Actions. Resolve registered services through the current context instead.

For services that themselves have constructor dependencies, register a factory (including `inject()` where appropriate) so object construction stays inside the service container.

---

# Dependency Graph

A typical Drogular request looks like this:

```text
HTTP Request
        │
        ▼
      Page
        │
        ▼
   Component
        │
        ▼
     Service
        │
        ▼
 Repository
```

Pages and Components resolve application services from the current render context; Actions resolve them from `ActionContext`. Registered services may in turn use constructor-injection factories.

---

# What's Next?

Now that you understand how services are created and provided, the next guide explains how requests are mapped to pages through Drogular's routing system.
