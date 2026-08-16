# Your First Drogular Application

In this guide, you'll turn the project from the installation guide into a small but complete Drogular application.

The example is intentionally simple, but every file uses the same APIs and boundaries as larger Drogular applications.

By the end, you'll have:

- a `Page` mounted at `/`
- a reusable template `Component`
- an application `Service` registered through dependency injection
- a real `main.cpp` showing startup order
- template files rendered by Drogular

---

# Before You Begin

Complete [Installation](installation.md) first. This guide assumes you already have:

```text
hello_drogular/
├── CMakeLists.txt
└── src/
    └── main.cpp
```

No additional CMake source files are required because the classes below are header-only for this first example.

---

# Final Project Layout

Create these files and directories:

```text
hello_drogular/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── home_service.hpp
│   ├── home_page.hpp
│   └── components/
│       └── home_message_component.hpp
└── templates/
    ├── home.html
    └── components/
        └── home_message.html
```

The request will flow through the application like this:

```text
GET /
  ↓
HomePage
  ↓
home.html
  ↓
<HomeMessage />
  ↓
HomeMessageComponent
  ↓
HomeService
```

---

# 1. Create the Service

Create `src/home_service.hpp`:

```cpp
#pragma once

#include <string>

class HomeService
{
public:
    std::string welcomeMessage() const
    {
        return "Welcome to Drogular";
    }
};
```

The service contains application logic and does not know anything about HTML, routes, or templates.

---

# 2. Create the Component

Create `src/components/home_message_component.hpp`:

```cpp
#pragma once

#include "../home_service.hpp"

#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

#include <string>

class HomeMessageComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "HomeMessage";

    void onInit(drogular::RenderContext& context) override
    {
        const auto service =
            context.requireService<HomeService>();

        context.set(
            "message",
            service->welcomeMessage()
        );
    }

    std::string templatePath() const override
    {
        return "components/home_message.html";
    }
};
```

Two includes matter here:

- `<drogular/component.hpp>` provides `TemplateComponent`.
- `<drogular/render_context.hpp>` provides the request render context and service resolution API.

Components are created by Drogular with their default constructor. Application services are resolved from the current context instead of being constructor-injected into Page or Component objects.

Now create `templates/components/home_message.html`:

```html
<p>{{ message }}</p>
```

---

# 3. Create the Page

Create `src/home_page.hpp`:

```cpp
#pragma once

#include <drogular/page.hpp>

#include <string>

class HomePage final : public drogular::TemplatePage
{
public:
    std::string templatePath() const override
    {
        return "home.html";
    }
};
```

`<drogular/page.hpp>` provides `Page` and `TemplatePage`.

Create `templates/home.html`:

```html
<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8" />
    <title>Hello Drogular</title>
</head>
<body>
    <h1>Hello Drogular</h1>
    <HomeMessage />
</body>
</html>
```

The uppercase `HomeMessage` tag is resolved through Drogular's component registry during rendering.

---

# 4. Organize `main.cpp`

Replace `src/main.cpp` with:

```cpp
#include "components/home_message_component.hpp"
#include "home_page.hpp"
#include "home_service.hpp"

#include <drogular/app.hpp>
#include <drogular/services.hpp>

int main()
{
    drogular::App app;

    // 1. Application-wide rendering configuration.
    app.templateRoot("templates");

    // 2. Register application services.
    app.services().add<HomeService>(
        drogular::ServiceLifetime::Singleton
    );

    // 3. Register reusable UI components.
    app.component<HomeMessageComponent>();

    // 4. Register HTTP entry points.
    app.page<HomePage>("/");

    // 5. Start the server only after configuration is complete.
    app.run(8080);

    return 0;
}
```

For a small application it is fine to keep startup configuration directly in `main.cpp`. The important part is the order:

```text
create App
    ↓
configure application options
    ↓
register services
    ↓
register components
    ↓
register pages and actions
    ↓
run
```

`app.run(...)` should be the last application operation because it starts the server and enters the runtime loop.

---

# 5. Build and Run

Build from the project root:

```bash
cmake --build build
```

Run the executable:

```bash
./build/hello_drogular
```

Then open:

```text
http://localhost:8080/
```

You should see:

```text
Hello Drogular
Welcome to Drogular
```

---

# What Drogular Did

When the browser requests `/`:

1. `App` routes the request to a fresh `HomePage` instance.
2. `HomePage` renders `templates/home.html`.
3. The component renderer finds `<HomeMessage />` and creates `HomeMessageComponent`.
4. `HomeMessageComponent::onInit()` resolves `HomeService` from the request context.
5. The component places `message` into its render context.
6. `components/home_message.html` renders the value.
7. The final HTML response is returned to the browser.

The service is application-scoped because it was registered as a `Singleton`; Page and Component instances themselves are created by the framework for rendering and should resolve registered services through their contexts.

---

# Include Rule for Documentation Examples

The first complete example of a Drogular type should show the headers required to compile it. Later snippets may omit repeated includes when they focus on one API detail.

As a practical rule in application code, include the Drogular header that declares the type you directly use:

```cpp
#include <drogular/app.hpp>            // drogular::App
#include <drogular/page.hpp>           // Page, TemplatePage
#include <drogular/component.hpp>      // Component, TemplateComponent
#include <drogular/render_context.hpp> // RenderContext
```

This keeps examples and application files understandable without relying on accidental transitive includes.

---

# What's Next?

Continue with [Project Structure](project-structure.md). It shows how to keep the same startup sequence while moving registration and configuration out of `main.cpp` as the application grows.
