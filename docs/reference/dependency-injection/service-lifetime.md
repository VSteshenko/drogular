# `ServiceLifetime`

**Namespace:** `drogular`  
**Header:** `<drogular/services.hpp>`  
**Kind:** Enum class

## Purpose

`ServiceLifetime` selects when a registered service is created and how long the created instance is reused.

---

## Values

```cpp
enum class ServiceLifetime {
    Singleton,
    LazySingleton,
    Transient,
    Scoped
};
```

### `Singleton`

The service is created during registration and stored in `ApplicationServices`.

Every later resolution returns the same shared instance.

### `LazySingleton`

The factory is stored during registration.

The first mutable `ApplicationServices::service<T>()` call creates and stores the instance. Concurrent first resolutions of the same service type are serialized, so one instance is created and published. Later mutable resolutions return the stored object.

The const `service<T>()` overload does not instantiate lazy services.

### `Transient`

The factory executes for every mutable `ApplicationServices::service<T>()` call.

Instances are not cached by the container.

### `Scoped`

`ApplicationServices` stores the factory, while request contexts own the instance cache.

`RenderContext` and `ActionContext` resolve scoped services through a request-owned `ServiceScope`. Child render contexts share their parent's scope. Repeated scoped resolution anywhere in the same request therefore returns the same instance, while a new HTTP request receives a new instance.

---

## Example

```cpp
app.services().add<Configuration>(
    drogular::ServiceLifetime::Singleton
);

app.services().addFactory<ExpensiveService>(
    drogular::ServiceLifetime::LazySingleton,
    [] {
        return std::make_shared<ExpensiveService>();
    }
);

app.services().addFactory<Formatter>(
    drogular::ServiceLifetime::Transient,
    [] {
        return std::make_shared<Formatter>();
    }
);

app.services().addFactory<RequestWorkspace>(
    drogular::ServiceLifetime::Scoped,
    [] {
        return std::make_shared<RequestWorkspace>();
    }
);
```

---

## Related Types

- [`ApplicationServices`](application-services.md)
- [`RenderContext`](../rendering/render-context.md)
