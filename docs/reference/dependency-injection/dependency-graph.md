# `DependencyGraph`

**Namespace:** `drogular`  
**Header:** `<drogular/dependency_graph.hpp>`  
**Kind:** Class

## Purpose

`DependencyGraph` stores declared service-to-service dependency metadata.

It does not resolve or construct services. The graph is used for inspection and for detecting missing registrations or circular dependencies.

---

## Synopsis

```cpp
class DependencyGraph {
public:
    template <typename ServiceType, typename DependencyType>
    void addDependency();

    template <typename ServiceType>
    std::vector<std::type_index> dependencies() const;

    std::vector<std::type_index> dependencies(
        std::type_index service
    ) const;

    bool dependsOn(
        std::type_index service,
        std::type_index dependency
    ) const;

    const std::unordered_map<
        std::type_index,
        std::vector<std::type_index>
    >& allDependencies() const;

    std::vector<std::type_index>
    circularDependencyPath() const;

    bool hasCircularDependencies() const;
};
```

---

## Recording Dependencies

### `addDependency<Service, Dependency>()`

```cpp
graph.addDependency<Service, Repository>();
```

Appends one directed dependency edge.

Duplicate edges are not removed automatically.

[`inject()`](inject.md) calls this method for each constructor dependency it declares.

---

## Reading Dependencies

### `dependencies<Service>()`

```cpp
auto dependencies =
    graph.dependencies<Service>();
```

Returns a copy of the direct dependencies recorded for the service.

### `dependencies(std::type_index)`

Returns a copy of the direct dependencies for a runtime service type.

Returns an empty vector when the service has no graph entry.

### `dependsOn()`

```cpp
bool dependsOn(
    std::type_index service,
    std::type_index dependency
) const;
```

Checks only direct dependency edges.

It does not perform a transitive dependency search.

### `allDependencies()`

Returns a const reference to the complete internal adjacency map.

---

## Cycle Detection

### `hasCircularDependencies()`

Returns `true` when the graph contains at least one cycle.

### `circularDependencyPath()`

Returns one detected traversal path when a cycle exists, or an empty vector otherwise.

The returned vector is diagnostic traversal data rather than a normalized minimal cycle representation.

---

## Behavior

`DependencyGraph` stores metadata only for dependencies explicitly added to it.

Manual factories do not automatically create graph edges:

```cpp
services.addFactory<Service>(
    [&services] {
        return std::make_shared<Service>(
            services.requireService<Repository>()
        );
    }
);
```

If dependency validation is required for such a factory, add the metadata manually or use [`inject()`](inject.md).

---

## Example

```cpp
drogular::DependencyGraph graph;

graph.addDependency<Service, Repository>();
graph.addDependency<Service, Logger>();

if (graph.dependsOn(
        std::type_index(typeid(Service)),
        std::type_index(typeid(Repository)))) {
    // Direct dependency exists.
}
```

---

## Thread Safety

`DependencyGraph` has no internal synchronization.

Populate it during application startup before concurrent reads begin.

---

## Related Types

- [`ApplicationServices`](application-services.md)
- [`DependencyValidationResult`](dependency-validation-result.md)
- [`inject()`](inject.md)
