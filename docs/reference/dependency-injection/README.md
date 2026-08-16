# Dependency Injection

Drogular uses `ApplicationServices` as the application-wide dependency injection container.

Services can be registered as singleton, lazy singleton, transient, or scoped. Constructor factories created with `inject()` also record dependency metadata that can be validated independently from service creation.

---

## Types

- [`ApplicationServices`](application-services.md) — registers and resolves application services.
- [`ServiceLifetime`](service-lifetime.md) — selects service creation and reuse behavior.
- [`ServiceRegistration`](service-registration.md) — describes one registered service for inspection.
- [`DependencyValidationResult`](dependency-validation-result.md) — contains dependency validation errors.
- [`DependencyGraph`](dependency-graph.md) — records declared service dependencies and detects cycles.
- [`inject()`](inject.md) — creates constructor-injection factories and records dependency metadata.

---

## Typical Flow

```text
Application startup
        │
        ▼
ApplicationServices
        │
        ├── Singleton / LazySingleton
        ├── Transient
        └── Scoped factory
                │
                ▼
          RenderContext
          scoped cache
                │
                ▼
             Service
```

Application-wide registrations live in `ApplicationServices`.

Scoped instances are different: `ApplicationServices` stores only the scoped factory, while each HTTP request owns a `ServiceScope`. `RenderContext` and `ActionContext` resolve through that scope, and child render contexts share it.

---

## Constructor Injection

`inject<Service, Dependencies...>()` creates a factory that resolves the listed dependencies through `ApplicationServices::requireService<T>()` and passes them to the service constructor.

It also records the declared dependency edges in `DependencyGraph`.

```cpp
app.services().add<Repository>();

app.services().addFactory<Service>(
    drogular::ServiceLifetime::Singleton,
    drogular::inject<Service, Repository>(
        app.services()
    )
);
```

---

## Validation

Dependency validation checks two conditions:

- every dependency recorded in `DependencyGraph` is registered;
- the dependency graph contains no cycle.

Validation is explicit through `ApplicationServices::validateDependencies()`.

---

## Getting Started

- [Dependency Injection](../../getting-started/dependency-injection.md)

## Related Reference

- [`App`](../application/app.md)
- [`RenderContext`](../rendering/render-context.md)
- [`ActionContext`](../actions/action-context.md)
