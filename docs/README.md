# Drogular Documentation

Welcome to the official Drogular documentation.

Drogular is a component-driven C++ web framework built on Drogon for server-rendered applications, GraphQL integration, progressive web applications, and extensible Developer Tools.

The documentation is organized by purpose. Start with the fundamentals, then continue with practical patterns, architectural decisions, or the public API.

---

# Documentation Structure

## 🚀 Getting Started

Learn how to install Drogular and build your first application.

Recommended for new users.

Topics include:

- Installation
- Your First Drogular Application
- Project Structure
- Components
- Dependency Injection
- Routing

[Open Getting Started](getting-started/README.md)

---

## 📖 Cookbook

Learn the architectural patterns and recommended practices used throughout Drogular applications.

Every article is based on the current framework implementation and working examples. The Cookbook documents existing solutions rather than future APIs.

Available guides:

- Forms & Validation
- State Management
- GraphQL
- Localization

[Open the Cookbook](cookbook/README.md)

---

## 🏗 Architecture

Understand why Drogular is designed the way it is.

This section focuses on design decisions and internal architecture rather than API usage.

Topics include:

- Framework Philosophy
- Feature Architecture
- Data Providers
- Template Rendering Pipeline
- Portal Feature Validation

[Open Architecture](architecture/README.md)

---

## 📚 API Reference

Find reference documentation for Drogular's supported public API.

This section describes public classes, methods, interfaces, and components.

[Open API Reference](reference/README.md)

---

## 🧩 Examples

The repository includes focused examples for individual framework capabilities and a complete reference application.

| Example | Demonstrates |
|---------|--------------|
| `todo_pwa` | Components, state management, forms, validation, and PWA support |
| `auth_sample` | Authentication and protected pages |
| `repository_sample` | Repository abstraction and dependency injection |
| `developer_tools` | Extending application inspection and Developer Tools |
| `portal_demo` | Complete reference application architecture |

`PortalDemo` is the primary application used to validate and evolve Drogular's public API before features are documented as established patterns.

---

# Learning Path

If you are new to Drogular, follow this order:

1. [Getting Started](getting-started/README.md)
2. Explore the focused examples
3. Study `PortalDemo`
4. Read the [Cookbook](cookbook/README.md)
5. Review the [Architecture](architecture/README.md)
6. Use the [API Reference](reference/README.md) when you need exact interface details

---

# Engineering Principles

Drogular is developed according to the principles in [`ENGINEERING.md`](../ENGINEERING.md).

Code and working examples are the source of truth. Features are validated in real applications before they are documented as established framework behavior.

---

# Contributing

Contributions are welcome.

Before contributing, read:

- [`ENGINEERING.md`](../ENGINEERING.md)
- [`CONTRIBUTING.md`](../CONTRIBUTING.md)

These documents describe the project's engineering philosophy and development workflow.
