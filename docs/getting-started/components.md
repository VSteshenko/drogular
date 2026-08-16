# Components

Components are reusable server-side UI building blocks in Drogular.

A component renders HTML and may prepare request-specific presentation data during its lifecycle. Business logic and infrastructure access should normally remain behind application services.

---

# A Minimal Template Component

A complete component header looks like this:

```cpp
#pragma once

#include "../services/user_service.hpp"

#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

#include <string>

class UserSummaryComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "UserSummary";

    void onInit(drogular::RenderContext& context) override
    {
        const auto users =
            context.requireService<UserService>();

        context.set("userCount", users->count());
    }

    std::string templatePath() const override
    {
        return "components/user_summary.html";
    }
};
```

Register it once during startup:

```cpp
app.component<UserSummaryComponent>();
```

Then use it from a template:

```html
<UserSummary />
```

---

# Lifecycle

Drogular runs the component lifecycle for each rendered instance:

```text
onInit(context)
    ↓
render(context)
    ↓
onDestroy(context)
```

`TemplateComponent::render()` performs template rendering, so most template components override `onInit()` and `templatePath()` rather than `render()` itself.

---

# Service Access

Page, Action, and Component types are created by their framework registries with default constructors. Drogular does **not** currently constructor-inject application services into Component objects.

Resolve required services from the current `RenderContext`:

```cpp
void onInit(drogular::RenderContext& context) override
{
    const auto service =
        context.requireService<HomeService>();

    context.set(
        "message",
        service->welcomeMessage()
    );
}
```

This keeps service lifetime management inside `ApplicationServices` while making request-scoped resolution explicit.

---

# Responsibilities

Typical component responsibilities include:

- rendering reusable HTML
- preparing presentation-specific values
- formatting application data for templates
- reading component inputs
- composing child components

Avoid placing database access, GraphQL transport details, authorization policy, or domain rules directly in UI components. Put those behind Services or providers and resolve the abstraction the component needs.

---

# Inputs

Registered component tags can receive string inputs from templates:

```html
<UserCard userId="42" />
```

A component reads an input with `input<T>()`:

```cpp
const auto userId =
    input<std::string>("userId")
        .value_or("");
```

Template expressions in input values are rendered before the component receives them.

---

# Local State and Shared State

An individual Component instance can hold local data needed during that render.

Do not assume mutable state stored in a shared Singleton service is automatically safe. Application-wide mutable state may be accessed by concurrent HTTP requests and must provide its own synchronization.

---

# Composition

Large interfaces should be built from smaller registered components:

```text
DashboardPage
│
├── HeaderComponent
├── NavigationComponent
├── StatisticsComponent
└── ActivityComponent
```

Composition is preferable to one large component that mixes unrelated presentation responsibilities.

---

# Include Guidance

For standalone examples, include the header that declares each Drogular type you directly use:

```cpp
#include <drogular/component.hpp>
#include <drogular/render_context.hpp>
```

Do not rely on another Drogular header happening to include these transitively.

---

# What's Next?

Continue with [Dependency Injection](dependency-injection.md) to learn how services are registered, resolved, and scoped.
