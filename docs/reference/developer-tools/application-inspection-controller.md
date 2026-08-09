# `ApplicationInspectionController`

**Namespace:** `drogular`  
**Header:** `<drogular/developer_tools/application_inspection_controller.hpp>`  
**Kind:** Controller class

## Purpose

`ApplicationInspectionController` exposes the current [`ApplicationInspection`](application-inspection.md) as JSON over HTTP.

Application code normally enables it through `App::enableInspection()` rather than constructing the controller directly.

## Path

```cpp
static constexpr const char* Path =
    "/__drogular/inspection";
```

The route is registered for HTTP GET.

## Construction

```cpp
explicit ApplicationInspectionController(
    ApplicationServices* services
);
```

The controller keeps a non-owning pointer to the application service container.

## `handle()`

```cpp
void handle(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&
    )>&& callback
) const;
```

The request itself is currently ignored.

The controller resolves `ApplicationInspectionProvider` from `ApplicationServices`.

If the provider is unavailable, it returns HTTP `503 Service Unavailable` with an empty response body.

Otherwise it calls the provider, serializes the returned snapshot with `toJson()`, and returns a JSON response.

## Provider Type

```cpp
using ApplicationInspectionProvider =
    std::function<ApplicationInspection()>;
```

`App::enableInspection()` registers a provider that calls `App::inspect()`.

## Errors

Exceptions from the inspection provider, contributors, or JSON construction are not handled inside `ApplicationInspectionController::handle()`.

## Lifetime

`App` owns the controller through a `std::shared_ptr` and captures that shared pointer in the Drogon handler registration.

The `ApplicationServices*` supplied to the constructor must remain valid for the controller lifetime.

## Related Types

- [`ApplicationInspection`](application-inspection.md)
- [`DeveloperToolsContributor`](developer-tools-contributor.md)
- [`ApplicationServices`](../dependency-injection/application-services.md)
- [`App`](../application/app.md)
