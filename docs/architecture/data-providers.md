# Data Provider Architecture

## Overview

Drogular encourages applications to separate business logic from data access.

Pages and Actions should depend only on Provider interfaces. Providers are responsible for business operations, while concrete implementations decide how data is stored or retrieved.

This architecture makes it possible to replace the data source without changing UI or application logic.

```text
Pages / Actions
        │
        ▼
Provider Interface
        │
        ├───────────────┐
        ▼               ▼
Memory Provider   GraphQL Provider
        │               │
        ▼               ▼
      Models        Documents
                        │
                        ▼
                  GraphQL Client
                        │
                        ▼
                     Transport
```
---

## Responsibilities

### Provider

A Provider exposes business operations.

It does not know about HTTP, GraphQL queries, JSON, SQL, or transport details.

Example:
```c++
class ProjectProvider {
public:
    virtual std::vector<Project> all() const = 0;
    virtual std::optional<Project> findById(int id) const = 0;
    virtual Project create(Project project) = 0;
    virtual bool update(Project project) = 0;
    virtual bool remove(int id) = 0;
};
```
Pages and Actions communicate only with Providers.

---

### Memory Provider

A Memory Provider is the simplest Provider implementation.

It stores application models directly in memory.

Example:
```text
ProjectProvider
↓
MemoryProjectProvider
↓
std::vector<Project>
```
Memory Providers are useful for:

- examples
- tests
- prototypes
- offline mode

---

### GraphQL Provider

A GraphQL Provider implements the same Provider interface but retrieves data through GraphQL.
```text
ProjectProvider
↓
GraphQLProjectProvider
↓
GraphQL Client
```
The application does not know which implementation is registered.

---

### Documents

Documents build GraphQL requests.

They contain GraphQL operations only.

Examples:
```c++
ProjectQueries::all()
ProjectQueries::findById(id)
ProjectMutations::create(project)
ProjectMutations::update(project)
ProjectMutations::remove(id)
```
Documents do not contain business logic.

---

### Mapper

A Mapper converts between application models and GraphQL data.

Responsibilities:

- GraphQL Result → Model
- Model → GraphQL Variables

A Mapper never performs network requests.

---

### GraphQL Client

The GraphQL Client executes GraphQL requests.

It knows nothing about application models.

Responsibilities:

- execute request
- return response
- transport
- serialization

---

## Dependency Rule

Dependencies always point downwards.
```text
Pages
↓
Provider
↓
Documents
↓
GraphQL Client
↓
Transport
```
Mapping is independent from transport.
```text
GraphQL Result
↓
Mapper
↓
Model
```
Providers connect these two independent flows.

---

## Model-Based APIs

Providers should exchange models instead of individual fields.

Preferred:
```c++
provider->update(project);
```
Instead of:
```c++
provider->update(
    id,
    title,
    status
);
```
Benefits:

- easier evolution of models
- fewer breaking API changes
- cleaner implementations
- simpler GraphQL integration

---

## Switching Data Sources

Changing the data source should require only a different service registration.

Development:
```c++
services.addFactory<ProjectProvider>(
    ServiceLifetime::Singleton,
    [] {
        return std::make_shared<MemoryProjectProvider>();
    }
);
```
Production:
```c++
services.addFactory<ProjectProvider>(
    ServiceLifetime::Singleton,
    [] {
        return std::make_shared<GraphQLProjectProvider>();
    }
);
```
No changes should be required in:

- Pages
- Actions
- Templates
- Business logic

---

## Design Principles

- Depend on Provider interfaces.
- Keep Providers transport-independent.
- Keep Documents GraphQL-specific.
- Keep Mappers focused on data transformation.
- Exchange models instead of parameter lists.
- Replace implementations through Dependency Injection.
- Validate architecture using real applications rather than hypothetical use cases.
