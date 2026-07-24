# Project Structure

This guide explains the recommended directory structure for Drogular applications.

The goal is to keep projects easy to navigate, maintain, and scale as they grow.

The same structure works for both small applications and large production systems.

---

# Recommended Structure

```text
src/
├── components/
├── pages/
├── services/
├── models/
├── stores/
├── actions/
├── graphql/
├── layouts/
├── middleware/
├── routing/
├── app/
└── main.cpp
```

Each directory has a single responsibility.

---

# pages/

The **pages** directory contains application entry points.

A page receives an incoming request, coordinates the required services, and returns the response.

Pages should remain lightweight and avoid implementing business logic.

Examples:

```text
pages/
├── HomePage
├── LoginPage
├── DashboardPage
```

---

# components/

Components are responsible for rendering the user interface.

They are reusable building blocks that can be composed to build complex pages.

Examples:

```text
components/
├── NavigationComponent
├── TodoListComponent
├── UserCardComponent
```

---

# services/

Services contain business logic.

Typical responsibilities include:

- database access
- GraphQL communication
- authentication
- validation
- domain logic

Services are provided through Drogular's dependency injection container.

---

# models/

Models represent application data.

Examples:

- User
- Todo
- Product
- Order

Models should not contain presentation logic.

---

# stores/

Stores manage application state.

They provide a predictable way to share state between components.

Not every application requires stores, but they become valuable as applications grow.

---

# actions/

Actions represent user intent.

Examples:

- CreateTodo
- DeleteTodo
- LoginUser
- LogoutUser

Separating actions from components helps keep user interfaces focused on presentation.

---

# graphql/

The **graphql** directory contains GraphQL operations.

Typical contents include:

- queries
- mutations
- fragments

Keeping GraphQL definitions together makes them easier to discover and maintain.

---

# layouts/

Layouts define the common structure shared by multiple pages.

Typical examples include:

- application layout
- authentication layout
- administration layout

Layouts help eliminate duplicated page structure.

---

# middleware/

Middleware contains request processing logic that executes before or after pages.

Typical responsibilities include:

- authentication
- logging
- localization
- request validation

---

# routing/

The **routing** directory contains route registration.

Keeping routing separate from pages provides a single place to understand the application's URL structure.

---

# app/

The **app** directory contains application-wide configuration.

Typical responsibilities include:

- dependency injection registration
- application configuration
- startup logic

---

# main.cpp

`main.cpp` is the application's entry point.

Its responsibility should remain minimal:

- create the application
- initialize Drogular
- start the server

Business logic should never be implemented here.

---

# Design Principles

This structure follows several engineering principles used throughout Drogular:

- One directory, one responsibility.
- Keep business logic out of pages.
- Keep presentation inside components.
- Prefer composition over duplication.
- Make project navigation predictable.

---

# What's Next?

In the next guide, you'll learn how components work and why they are the primary building blocks of every Drogular application.
