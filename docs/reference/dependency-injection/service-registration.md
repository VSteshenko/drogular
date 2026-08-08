# `ServiceRegistration`

**Namespace:** `drogular`  
**Header:** `<drogular/services.hpp>`  
**Kind:** Struct

## Purpose

`ServiceRegistration` is an inspection record describing one service registration known to `ApplicationServices`.

It is returned by `ApplicationServices::registrations()` and is primarily useful for diagnostics and tooling.

---

## Synopsis

```cpp
struct ServiceRegistration {
    std::string type;
    ServiceLifetime lifetime = ServiceLifetime::Singleton;
    bool instantiated = false;
};
```

---

## Members

### `type`

Runtime type-name string produced by `std::type_index::name()` for the registered service type.

The representation is implementation-defined and may be mangled.

### `lifetime`

The configured [`ServiceLifetime`](service-lifetime.md).

### `instantiated`

`true` when the service type currently has an entry in the instantiated singleton map.

This is normally true for singletons and becomes true for a lazy singleton after its first mutable resolution.

Transient and scoped services are not stored in that map, so their registration records remain uninstantiated.

---

## Related Types

- [`ApplicationServices`](application-services.md)
- [`ServiceLifetime`](service-lifetime.md)
