# `ApplicationServices`

**Namespace:** `drogular`  
**Header:** `<drogular/services.hpp>`  
**Kind:** Class

## Purpose

`ApplicationServices` is Drogular's application-wide service container.

It stores service registrations, resolves application services, owns dependency metadata, exposes the component registry, and provides framework-wide objects such as the configured GraphQL client and application options.

---

## Role in Drogular

`ApplicationServices` is the dependency-injection hub shared by the application.

```text
                    App
                     │
                     ▼
           ApplicationServices
              │      │      │
              │      │      └── DependencyGraph
              │      │
              │      ├── Singleton / Lazy / Transient
              │      │
              │      └── Scoped factories
              │               │
              │               ▼
              │         RenderContext
              │         scoped cache
              │
              ├── ComponentRegistry
              ├── ApplicationOptions
              └── GraphQLClient
```

`App::services()` exposes the container during startup.

Pages, actions, and components normally resolve services through their request/render contexts rather than keeping their own container reference.

Scoped lifetime is split across two layers: `ApplicationServices` owns the factory, while a mutable `RenderContext` creates and caches the scoped instance.

---

## Synopsis

```cpp
class ApplicationServices {
public:
    void setGraphQLClient(std::shared_ptr<GraphQLClient> client);
    GraphQLClient* graphQLClient() const;

    template <typename T>
    void registerService(std::shared_ptr<T> service);

    template <typename T>
    std::shared_ptr<T> service();

    template <typename T>
    std::shared_ptr<T> service() const;

    template <typename T>
    std::shared_ptr<T> requireService();

    template <typename T, typename... Args>
    std::shared_ptr<T> add(Args&&... args);

    template <typename T, typename... Args>
    std::shared_ptr<T> add(ServiceLifetime lifetime, Args&&... args);

    template <typename T>
    std::shared_ptr<T> addFactory(
        std::function<std::shared_ptr<T>()> factory
    );

    template <typename T>
    std::shared_ptr<T> addFactory(
        ServiceLifetime lifetime,
        std::function<std::shared_ptr<T>()> factory
    );

    template <typename T>
    void addLazy(std::function<std::shared_ptr<T>()> factory);

    template <typename T>
    void addTransient(std::function<std::shared_ptr<T>()> factory);

    template <typename T>
    void addScoped(std::function<std::shared_ptr<T>()> factory);

    template <typename T>
    std::shared_ptr<T> createScoped();

    bool hasService(std::type_index type) const;

    DependencyGraph& dependencyGraph();
    const DependencyGraph& dependencyGraph() const;

    DependencyValidationResult validateDependencies() const;

    ComponentRegistry& components();
    const ComponentRegistry& components() const;

    std::vector<ServiceRegistration> registrations() const;

    void setOptions(ApplicationOptions* options);
    ApplicationOptions* options();
    const ApplicationOptions* options() const;

    TemplateSourceCache& templateSourceCache();
    const TemplateSourceCache& templateSourceCache() const;
};
```

---

## Registering Services

### `registerService<T>()`

```cpp
template <typename T>
void registerService(
    std::shared_ptr<T> service
);
```

Stores the supplied pointer as a singleton registration for `T`.

The method does not reject a null pointer and does not clear older factories registered for the same type. Avoid re-registering the same type through different registration APIs.

### `add<T>()`

```cpp
template <typename T, typename... Args>
std::shared_ptr<T> add(Args&&... args);
```

Constructs `T` immediately with `std::make_shared<T>()`, registers it as a singleton, and returns the created instance.

```cpp
app.services().add<RepositorySampleUserRepository>();
```

### `add<T>(ServiceLifetime, ...)`

```cpp
template <typename T, typename... Args>
std::shared_ptr<T> add(
    ServiceLifetime lifetime,
    Args&&... args
);
```

Registers `T` with the selected lifetime.

For `Singleton`, the object is constructed immediately and returned.

For `LazySingleton`, `Transient`, and `Scoped`, the method stores a factory and returns `nullptr`.

### `addFactory<T>()`

```cpp
template <typename T>
std::shared_ptr<T> addFactory(
    std::function<std::shared_ptr<T>()> factory
);
```

Executes the factory immediately and registers the returned service as a singleton.

Throws `std::runtime_error` when the factory returns `nullptr`.

### `addFactory<T>(ServiceLifetime, ...)`

```cpp
template <typename T>
std::shared_ptr<T> addFactory(
    ServiceLifetime lifetime,
    std::function<std::shared_ptr<T>()> factory
);
```

Dispatches to the lifetime-specific registration behavior.

Only `Singleton` returns the created instance immediately. Other lifetimes return `nullptr` because creation is deferred or scope-dependent.

### `addLazy<T>()`

```cpp
template <typename T>
void addLazy(
    std::function<std::shared_ptr<T>()> factory
);
```

Stores a lazy-singleton factory.

The first mutable `service<T>()` call executes the factory and stores the result in the singleton map. Later calls return the same object.

Throws `std::runtime_error` if the factory returns `nullptr`.

### `addTransient<T>()`

```cpp
template <typename T>
void addTransient(
    std::function<std::shared_ptr<T>()> factory
);
```

Stores a transient factory.

Each mutable `service<T>()` call creates a new instance.

### `addScoped<T>()`

```cpp
template <typename T>
void addScoped(
    std::function<std::shared_ptr<T>()> factory
);
```

Stores a scoped factory.

`ApplicationServices::service<T>()` does not resolve scoped registrations. A scope owner must call `createScoped<T>()` and cache the result. Drogular's mutable `RenderContext::service<T>()` performs this automatically.

### `createScoped<T>()`

```cpp
template <typename T>
std::shared_ptr<T> createScoped();
```

Executes the registered scoped factory for `T` and returns a new instance.

Returns `nullptr` when no scoped factory exists.

`ApplicationServices` itself does not cache the returned instance.

---

## Resolving Services

### Mutable `service<T>()`

```cpp
template <typename T>
std::shared_ptr<T> service();
```

Resolution order is:

1. already instantiated service;
2. lazy-singleton factory;
3. transient factory;
4. otherwise `nullptr`.

Scoped factories are intentionally not resolved here.

### Const `service<T>()`

```cpp
template <typename T>
std::shared_ptr<T> service() const;
```

The const overload only reads the instantiated service map.

It does not instantiate lazy singletons and does not resolve transient or scoped factories.

### `requireService<T>()`

```cpp
template <typename T>
std::shared_ptr<T> requireService();
```

Uses mutable `service<T>()` and throws `std::runtime_error` when resolution returns `nullptr`.

---

## Service Lifetimes

Drogular supports four lifetimes through [`ServiceLifetime`](service-lifetime.md):

| Lifetime | Creation | Reuse |
| --- | --- | --- |
| `Singleton` | during registration | application-wide |
| `LazySingleton` | first mutable resolution | application-wide |
| `Transient` | every mutable resolution | never cached |
| `Scoped` | when a scope requests it | cached by the scope, not by `ApplicationServices` |

For request rendering, [`RenderContext`](../rendering/render-context.md) is the scope owner used by the current implementation.

---

## Dependency Metadata

### `dependencyGraph()`

```cpp
DependencyGraph& dependencyGraph();
const DependencyGraph& dependencyGraph() const;
```

Returns the graph used to record declared constructor dependencies.

[`inject()`](inject.md) populates this graph automatically.

### `validateDependencies()`

```cpp
DependencyValidationResult validateDependencies() const;
```

Checks recorded dependency metadata.

Validation reports an error when:

- a recorded dependency type is not registered through any supported lifetime;
- the dependency graph contains at least one cycle.

Validation does not instantiate services and does not verify that factories can execute successfully.

The current error strings are intentionally generic and do not include type names.

---

## Registration Inspection

### `hasService()`

```cpp
bool hasService(
    std::type_index type
) const;
```

Returns `true` when the type exists in any singleton, lazy-singleton, transient, or scoped registration store.

### `registrations()`

```cpp
std::vector<ServiceRegistration> registrations() const;
```

Returns one [`ServiceRegistration`](service-registration.md) per entry in the lifetime table, sorted by the runtime type-name string.

For lazy singletons, `instantiated` becomes `true` after the first mutable resolution because the created object is stored in the singleton map.

---

## Framework-Owned Resources

### GraphQL client

```cpp
void setGraphQLClient(
    std::shared_ptr<GraphQLClient> client
);

GraphQLClient* graphQLClient() const;
```

Stores the application GraphQL client and returns it as a non-owning raw pointer.

### Components

```cpp
ComponentRegistry& components();
const ComponentRegistry& components() const;
```

Returns the application's [`ComponentRegistry`](../components/component-registry.md).

### Application options

```cpp
void setOptions(ApplicationOptions* options);
ApplicationOptions* options();
const ApplicationOptions* options() const;
```

Stores a non-owning pointer to [`ApplicationOptions`](../application/application-options.md).

When a non-null options pointer is assigned, the internal `TemplateSourceCache` loader is replaced using the current `templateRoot()`.

### Template source cache

```cpp
TemplateSourceCache& templateSourceCache();
const TemplateSourceCache& templateSourceCache() const;
```

Returns the container-owned template source cache.

`TemplateSourceCache` synchronizes its internal source map and loader. Concurrent cached reads use shared locking; cache misses, `clear()` and `setLoader()` use exclusive locking. Loaded sources are returned by value, so cache invalidation does not invalidate data already returned to a renderer.

The cache itself will be documented in the Rendering/Template reference when that API is added.

---

## Lifetime and Thread Safety

`ApplicationServices` is application-wide mutable state.

The implementation uses standard containers without internal locking. Register services during application startup, before concurrent request processing begins.

Runtime mutation or first-time lazy-singleton resolution from multiple threads requires external synchronization if concurrent access is possible.

Transient and scoped factories may also be invoked concurrently by different requests and should be safe for that usage.

---

## Example

Constructor injection with a singleton repository:

```cpp
class UserService {
public:
    explicit UserService(
        std::shared_ptr<UserRepository> repository
    )
        : repository_(std::move(repository))
    {
    }

private:
    std::shared_ptr<UserRepository> repository_;
};

app.services().add<UserRepository>();

app.services().addFactory<UserService>(
    drogular::ServiceLifetime::Singleton,
    drogular::inject<UserService, UserRepository>(
        app.services()
    )
);
```

Resolve the service from a render context:

```cpp
auto service =
    context.requireService<UserService>();
```

---

## Related Types

- [`SessionStore`](../authentication-and-sessions/session-store.md)
- [`ServiceLifetime`](service-lifetime.md)
- [`ServiceRegistration`](service-registration.md)
- [`DependencyGraph`](dependency-graph.md)
- [`DependencyValidationResult`](dependency-validation-result.md)
- [`inject()`](inject.md)
- [`RenderContext`](../rendering/render-context.md)
- [`ActionContext`](../actions/action-context.md)
- [`App`](../application/app.md)

## See Also

- [Getting Started: Dependency Injection](../../getting-started/dependency-injection.md)
