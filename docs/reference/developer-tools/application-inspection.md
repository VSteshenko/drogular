# `ApplicationInspection`

**Namespace:** `drogular`  
**Header:** `<drogular/developer_tools/application_inspection.hpp>`  
**Kind:** Data structure

## Purpose

`ApplicationInspection` is the in-memory representation of Drogular's application inspection contract.

It contains the framework surface that Developer Tools and external tooling can inspect without depending on implementation-specific application objects.

## Role in Drogular

`ApplicationInspection` is the boundary between a running Drogular application and developer tooling.

```text
App::inspect()
     │
     ▼
ApplicationInspection
     │
     ├── routes
     ├── components
     ├── services
     ├── diagnostics
     └── custom sections
            │
            ▼
        toJson()
            │
      ┌─────┴─────┐
      ▼           ▼
Diagnostics UI  IDE / CLI
```

The browser diagnostics application is only one consumer. The JSON contract can be consumed independently by other tools.

## Synopsis

```cpp
struct ApplicationInspection {
    static constexpr int SchemaVersion = 3;

    std::vector<RouteInspection> routes;
    std::vector<ComponentInspection> components;
    std::vector<ServiceInspection> services;
    std::vector<Diagnostic> diagnostics;
    std::vector<InspectionSection> sections;

    void addSection(InspectionSection section);
};
```

## Inspection Data Types

### `RouteKind`

```cpp
enum class RouteKind {
    Page,
    Action,
    StaticFiles,
    ServiceWorker,
    OfflinePage,
    Inspection
};
```

`toString(RouteKind)` serializes these values as `page`, `action`, `static-files`, `service-worker`, `offline-page`, and `inspection`.

### `RouteInspection`

```cpp
struct RouteInspection {
    std::string path;
    RouteKind kind = RouteKind::Page;
    std::string method;
    std::string target;
};
```

Describes one inspected application route.

### `ComponentInspection`

```cpp
struct ComponentInspection {
    std::string tag;
};
```

Describes a registered component tag.

### `ServiceInspection`

```cpp
using ServiceInspection = ServiceRegistration;
```

Reuses the dependency-injection registration descriptor and therefore exposes service type name, lifetime, and instantiated state.

### `InspectionSection`

```cpp
struct InspectionSection {
    std::string id;
    std::string title;
    std::string component;
    Json::Value data{Json::objectValue};
};
```

Represents an application-defined extension section.

`component` is a semantic renderer name. It is intentionally independent from the browser module URL registered through [`DeveloperToolsComponentRegistry`](developer-tools-component-registry.md).

## `addSection()`

```cpp
void addSection(InspectionSection section);
```

Adds or replaces a custom section.

- `section.id` must not be empty; otherwise `std::invalid_argument` is thrown.
- Adding another section with the same id replaces the existing section in place.
- An empty title is accepted; JSON serialization later falls back to the section id.

## JSON Contract

```cpp
Json::Value toJson(
    const ApplicationInspection& inspection
);
```

Serializes the inspection to the stable Developer Tools JSON contract.

The root object contains:

```text
schemaVersion
routes
components
services
diagnostics
sections
```

`SchemaVersion` is currently `3`.

`sections` always begins with four built-in sections:

1. `routes` using component `drogular.routes`
2. `components` using component `drogular.components`
3. `services` using component `drogular.services`
4. `diagnostics` using component `drogular.diagnostics`

Application-defined sections are appended afterwards.

## Creating an Inspection

The normal application entry point is:

```cpp
const auto inspection = app.inspect();
```

`App::inspect()` collects:

- registered page and action routes;
- configured static-file mappings;
- service worker and offline-page routes;
- component tags;
- DI service registrations;
- component diagnostics;
- inspection endpoint routes when enabled;
- contributions from [`DeveloperToolsContributor`](developer-tools-contributor.md) instances.

Calling `inspect()` does not start the server.

## Behavior

`ApplicationInspection` is a snapshot. It does not hold live references to router, service-container, or component-registry state.

Contributor execution occurs while the snapshot is being built, so contributor behavior directly affects `App::inspect()` and the inspection HTTP endpoint.

## Example

```cpp
const auto inspection = app.inspect();
const auto json = drogular::toJson(inspection);

std::cout
    << json["schemaVersion"].asInt()
    << '\n';
```

## Related Types

- [`DeveloperToolsContributor`](developer-tools-contributor.md)
- [`DeveloperToolsComponentRegistry`](developer-tools-component-registry.md)
- [`ApplicationInspectionController`](application-inspection-controller.md)
- [`Diagnostics`](diagnostics.md)
- [`App`](../application/app.md)
