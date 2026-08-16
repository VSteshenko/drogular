# `inject()`

**Namespace:** `drogular`  
**Header:** `<drogular/inject.hpp>`  
**Kind:** Function template

## Purpose

`inject<ServiceType, Dependencies...>()` creates a constructor-injection factory for `ServiceType`.

The function also records dependency metadata in the container's `DependencyGraph`.

---

## Synopsis

```cpp
template <typename ServiceType, typename... Dependencies>
auto inject(
    ApplicationServices& services
);
```

The returned callable constructs:

```cpp
std::make_shared<ServiceType>(
    services.requireService<Dependencies>()...
);
```

---

## Behavior

Calling `inject()` immediately records one dependency edge for every type in `Dependencies...`.

The returned factory captures `ApplicationServices` by reference. The container must therefore outlive the factory and every registration that stores it.

Dependencies are resolved when the factory executes, not when `inject()` itself is called.

Missing dependencies cause `ApplicationServices::requireService<T>()` to throw `std::runtime_error` when the factory executes.

---

## Example

```cpp
class Repository {
};

class UserService {
public:
    explicit UserService(
        std::shared_ptr<Repository> repository
    )
        : repository_(std::move(repository))
    {
    }

private:
    std::shared_ptr<Repository> repository_;
};

app.services().add<Repository>();

app.services().addFactory<UserService>(
    drogular::ServiceLifetime::Singleton,
    drogular::inject<UserService, Repository>(
        app.services()
    )
);
```

The same call records:

```text
UserService ──▶ Repository
```

in the container's [`DependencyGraph`](dependency-graph.md).

---

## Multiple Dependencies

```cpp
app.services().addFactory<Service>(
    drogular::ServiceLifetime::Singleton,
    drogular::inject<
        Service,
        Repository,
        Logger
    >(app.services())
);
```

The constructor receives the dependencies in the same order as the template argument list.

---

## Scoped Dependencies

`inject()` resolves dependencies directly through `ApplicationServices::requireService<T>()`.

That resolver does not create scoped services. Therefore, a service factory built with `inject()` cannot currently resolve a dependency registered only as `ServiceLifetime::Scoped` through this path.

Scoped resolution is performed by request contexts (`RenderContext` and `ActionContext`) through their `ServiceScope`. `inject()` itself still resolves directly from `ApplicationServices`, so constructor-injected dependencies of a factory cannot currently be scoped unless a scope-aware factory is provided explicitly.

---

## Related Types

- [`ApplicationServices`](application-services.md)
- [`DependencyGraph`](dependency-graph.md)
- [`ServiceLifetime`](service-lifetime.md)
