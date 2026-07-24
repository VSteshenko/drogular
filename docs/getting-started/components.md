# Components

Components are the primary building blocks of every Drogular application.

A component is responsible for rendering the user interface and handling user interaction.

Components should remain focused on the presentation layer. Business logic belongs elsewhere.

---

# Responsibilities

A component owns the user interface.

Typical responsibilities include:

- rendering HTML
- handling user interaction
- managing local UI state
- formatting data for display
- composing other components

A component should be easy to understand by reading it in isolation.

---

# Components Are Not Services

A common mistake is allowing components to become responsible for business logic.

In Drogular, responsibilities are intentionally separated.

| Components | Services |
|------------|----------|
| Render UI | Implement business logic |
| Handle user interaction | Access databases |
| Manage local UI state | Execute GraphQL operations |
| Compose other components | Apply business rules |
| Format data for presentation | Coordinate external systems |

This separation keeps components small, reusable, and easy to test.

---

# Dependency Injection

Components should never construct their own dependencies.

Instead, required services are provided by Drogular's dependency injection container.

```cpp
class HomeComponent : public Component
{
public:
    explicit HomeComponent(HomeService& service);

private:
    HomeService& service_;
};
```

This keeps dependencies explicit while allowing components to focus entirely on presentation.

---

# Local State

Components may own local UI state.

Examples include:

- selected tab
- expanded panel
- dialog visibility
- current page in a pager
- active menu item
- search text before submission

Local state exists only to support the user interface.

If state must be shared across multiple components, consider using a Store instead.

---

# User Interaction

Components react to user actions.

Examples include:

- button clicks
- form input
- checkbox changes
- keyboard shortcuts
- drag and drop

A component decides **what happened**, but not necessarily **what it means**.

Complex operations should be delegated to Actions or Services.

---

# Composition

Large interfaces should be built by composing smaller components.

For example:

```text
DashboardPage
│
├── HeaderComponent
├── NavigationComponent
├── StatisticsComponent
├── ActivityComponent
└── FooterComponent
```

Each component should have a single, well-defined responsibility.

Composition is preferred over creating large monolithic components.

---

# Business Logic

Components should never contain business logic.

Avoid code that:

- accesses databases
- performs GraphQL requests
- implements authorization rules
- validates business constraints
- communicates directly with external systems

These responsibilities belong to Services.

---

# A Good Component

A well-designed component is:

- focused
- reusable
- predictable
- easy to test
- easy to compose

It should answer one question:

> **How should this part of the user interface behave?**

It should never answer:

> **How does the business work?**

---

# Design Principle

A simple rule applies throughout Drogular:

> **Components own UI. Services own business logic.**

Keeping this boundary clear makes applications easier to understand, extend, and maintain.

---

# What's Next?

Now that you've learned how components are structured, the next guide explains how Drogular creates and provides services through its dependency injection container.
