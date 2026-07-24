# Dependency Injection

Dependency Injection is the standard way to create and provide services in Drogular applications.

All official examples and documentation use dependency injection.

Although C++ allows objects to be created manually, Drogular applications should rely on the dependency injection container whenever a class has dependencies.

---

# Why Dependency Injection?

Without dependency injection, classes become responsible for constructing the objects they need.

This increases coupling and makes applications harder to test, maintain, and extend.

With dependency injection, a class declares its dependencies, and Drogular provides them automatically.

---

# Constructor Injection

Dependencies are provided through the constructor.

```cpp
class HomeComponent : public Component
{
public:
    explicit HomeComponent(HomeService& service);

private:
    HomeService& service_;
};
```

The component does not create the service.

It simply declares that it requires one.

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

A single instance is shared within the current scope.

For web applications, this is typically the lifetime of a single request.

Scoped services are useful for request-specific state and resources.

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
| Scoped | Request-specific services |
| Transient | Lightweight stateless services |

Most services naturally fit into one of these four categories.

---

# Best Practices

- Inject dependencies through constructors.
- Keep dependencies explicit.
- Register services during application startup.
- Choose the narrowest appropriate lifetime.
- Avoid global objects and service locators.

---

# Avoid Manual Construction

Avoid creating services directly inside components or pages.

Instead of:

```cpp
auto service = std::make_shared<HomeService>();
```

prefer constructor injection.

Keeping object creation inside the dependency injection container ensures a consistent architecture throughout the application.

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

Each object receives the dependencies it requires.

No class is responsible for constructing the next layer.

---

# What's Next?

Now that you understand how services are created and provided, the next guide explains how requests are mapped to pages through Drogular's routing system.
