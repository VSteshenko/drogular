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

`ApplicationServices` stores the factory but does not cache or resolve the instance through `service<T>()`.

A scope owner calls `createScoped<T>()` and decides how long to reuse the result. In the current rendering pipeline, mutable `RenderContext::service<T>()` creates and caches one scoped instance per context.

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
