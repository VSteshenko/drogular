# Your First Drogular Application

In this guide, you'll build your first Drogular application.

Although the application is intentionally simple, it already uses the same architectural concepts found in larger Drogular projects:

- Pages
- Components
- Dependency Injection
- Routing

By the end of this guide, you'll have a working application and a solid foundation for everything that follows.

---

# Before You Begin

Make sure you have completed the installation guide.

Requirements:

- Drogular is installed
- The project builds successfully
- You are familiar with basic CMake usage

---

# What You'll Build

The application consists of three parts:

```
Application
│
├── HomePage
│
├── HomeComponent
│
└── HomeService
```

Each part has a single responsibility.

- **HomePage** handles the incoming request.
- **HomeComponent** renders the user interface.
- **HomeService** provides the data displayed by the page.

The service is created automatically by Drogular's dependency injection container.

---

# Create the Service

Begin by creating a simple service.

```cpp
class HomeService
{
public:
    std::string welcomeMessage() const;
};
```

For now, the service simply returns a welcome message.

In real applications, services typically communicate with databases, GraphQL APIs, or other backend systems.

---

# Create the Component

Next, create a component responsible for rendering the page.

```cpp
class HomeComponent : public Component
{
public:
    explicit HomeComponent(HomeService& service);

private:
    HomeService& service_;
};
```

The component receives the service through dependency injection.

Notice that the component does **not** create the service itself.

---

# Create the Page

Now create the application's first page.

```cpp
class HomePage : public Page
{
public:
    Response get(RequestContext&);
};
```

The page coordinates the request and renders the component.

---

# Register the Route

Register the page with the router.

```text
GET /
    ↓
HomePage
```

Opening the root URL now displays your first Drogular page.

---

# Build and Run

Build the project.

```bash
cmake --build build
```

Start the application.

Open your browser and navigate to:

```
http://localhost:8080/
```

You should see your application's home page.

---

# What You've Built

Although the application is intentionally small, it already demonstrates the core architecture of Drogular.

```
HTTP Request
        │
        ▼
     HomePage
        │
        ▼
 HomeComponent
        │
        ▼
  HomeService
```

This same flow scales naturally from small projects to large applications.

---

# What's Next?

In the next guide, you'll explore the project structure and learn where each part of a Drogular application belongs.

You'll also learn why Drogular separates pages, components, and services, and how this organization helps keep applications maintainable as they grow.
