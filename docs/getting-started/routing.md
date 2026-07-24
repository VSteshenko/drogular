# Routing

Routing connects incoming HTTP requests to pages.

Every request received by a Drogular application is matched to a route, which determines which page should handle the request.

---

# Routes

A route consists of two parts:

- an HTTP method
- a URL path

For example:

```text
GET /
        ↓
HomePage
```

When a client sends a `GET` request to `/`, Drogular invokes `HomePage`.

---

# Pages Own Routes

Routes should point to pages.

Pages are the application's entry points and coordinate the work required to generate a response.

A page may:

- receive request parameters
- resolve services through dependency injection
- create components
- return the final response

Components and services should never be registered as routes.

---

# Route Registration

Routes are registered during application startup.

Keeping all route registrations together makes the application's URL structure easy to understand and maintain.

For example:

```text
/
├── HomePage
├── AboutPage
├── LoginPage
└── DashboardPage
```

---

# Route Parameters

Routes may contain parameters.

For example:

```text
/users/{id}
```

When a request matches the route, the parameter is passed to the page.

The page is responsible for validating and using the parameter.

---

# HTTP Methods

Different HTTP methods usually represent different intentions.

Typical examples include:

| Method | Purpose |
|---------|---------|
| GET | Read data |
| POST | Create resources |
| PUT | Replace resources |
| PATCH | Update resources |
| DELETE | Remove resources |

Using the appropriate HTTP method makes APIs easier to understand.

---

# Keep Pages Thin

Pages should coordinate work rather than implement business logic.

A typical request flow looks like this:

```text
HTTP Request
        │
        ▼
      Route
        │
        ▼
      Page
        │
        ▼
     Service
        │
        ▼
   Component
        │
        ▼
 HTTP Response
```

Each layer has a single responsibility.

---

# Organizing Routes

As applications grow, routes should remain organized.

A common approach is grouping related routes together.

For example:

```text
/
├── Home
├── Authentication
│   ├── Login
│   └── Logout
├── Users
├── Products
└── Administration
```

This organization makes navigation predictable for both developers and users.

---

# Best Practices

- Register routes in a single location.
- Route requests only to pages.
- Keep pages focused on request coordination.
- Delegate business logic to services.
- Return components responsible for presentation.

---

# What's Next?

You now understand how requests reach your application.

The next guide explains how Pages coordinate services and components to produce the final response.
