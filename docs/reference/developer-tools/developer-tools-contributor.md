# `DeveloperToolsContributor`

**Namespace:** `drogular`  
**Header:** `<drogular/developer_tools/application_inspection.hpp>`  
**Kind:** Abstract interface

## Purpose

`DeveloperToolsContributor` lets an application add domain-specific data to an [`ApplicationInspection`](application-inspection.md) snapshot.

It extends inspection data without coupling the application to the built-in diagnostics UI.

## Public API

```cpp
class DeveloperToolsContributor {
public:
    virtual ~DeveloperToolsContributor() = default;

    virtual void contribute(
        ApplicationInspection& inspection
    ) const = 0;
};
```

Implement `contribute()` and add one or more custom sections with `ApplicationInspection::addSection()`.

## Example

```cpp
class SystemInfoContributor final
    : public drogular::DeveloperToolsContributor {
public:
    void contribute(
        drogular::ApplicationInspection& inspection
    ) const override {
        Json::Value data(Json::objectValue);
        data["profile"] = "development";

        inspection.addSection({
            .id = "system-info",
            .title = "System Info",
            .component = "example.system-info",
            .data = std::move(data)
        });
    }
};
```

Register it with `App`:

```cpp
app.developerToolsContributor(
    std::make_shared<SystemInfoContributor>()
);
```

If the section needs a custom browser renderer, register that renderer separately:

```cpp
app.developerToolsComponent(
    "example.system-info",
    "/assets/system-info.js"
);
```

## Behavior

Contributors run in registration order whenever an inspection snapshot is built.

Exceptions thrown by a contributor are not caught by `DeveloperToolsContributors::contribute()` and therefore propagate to the caller.

The interface itself does not require Developer Tools to be enabled. Registering a contributor creates the contributor collection service when necessary, but it does not automatically register the inspection endpoint or diagnostics page.

## Compatibility Alias

```cpp
using InspectionContributor = DeveloperToolsContributor;
```

The alias preserves the name used by the initial inspection API.

---

# `DeveloperToolsContributors`

**Namespace:** `drogular`  
**Header:** `<drogular/developer_tools/application_inspection.hpp>`  
**Kind:** Collection class

`DeveloperToolsContributors` owns the ordered list of registered contributor objects.

## Public API

```cpp
void add(
    std::shared_ptr<DeveloperToolsContributor> contributor
);

const std::vector<
    std::shared_ptr<DeveloperToolsContributor>
>& entries() const;

void contribute(
    ApplicationInspection& inspection
) const;
```

`add()` rejects `nullptr` with `std::invalid_argument`.

`contribute()` calls every registered contributor in order.

## Compatibility Alias

```cpp
using InspectionContributors = DeveloperToolsContributors;
```

## Thread Safety

The collection has no internal synchronization. Configure contributors during application startup before concurrent request handling begins.

## Related Types

- [`ApplicationInspection`](application-inspection.md)
- [`DeveloperToolsComponentRegistry`](developer-tools-component-registry.md)
- [`App`](../application/app.md)
- [`ApplicationServices`](../dependency-injection/application-services.md)
