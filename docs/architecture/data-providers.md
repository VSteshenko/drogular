# Data Provider Architecture

## Overview

Drogular applications should separate UI, business actions, data access, and data representation.

Pages and Actions depend on Provider interfaces. Providers expose business operations using application models. Concrete implementations decide where data comes from: memory, GraphQL, dataset-backed clients, or a real transport.

Portal Demo currently uses this architecture to switch data sources without changing Pages, Actions, Templates, or business logic.

```text
    Pages / Actions
            │
            ▼
    Provider Interface
            │
            ▼
    Provider Implementation
            │
            ▼
    GraphQL Client / Dataset / Transport
            │
            ▼
          Models
```

## Core Idea

The application works with models.
```c++
    PortalUser
    PortalProject
    PortalRole
    PortalProjectType
```
Providers exchange complete models instead of long parameter lists.

Preferred:
```c++
    provider->update(project);
```
Instead of:
```c++
    provider->update(id, title, status, ownerId, projectTypeId);
```
This keeps APIs stable when models evolve.

## Dataset

`PortalDataset` is the in-memory domain state used by Portal Demo, tests, and dataset-backed GraphQL clients.

It stores application models directly:
```c++
    users
    roles
    projects
    projectTypes
```
The Dataset is not tied to GraphQL, HTTP, JSON, or UI. It is the canonical demo/test state.

## Schema

`PortalSchema` describes model fields once.

Example:
```c++
    PortalSchema::projects()
        .key("id", &PortalProject::id)
        .required("title", &PortalProject::title)
        .required("status", &PortalProject::status)
        .reference("ownerId", &PortalProject::ownerId, "users", "id")
        .reference("projectTypeId", &PortalProject::projectTypeId, "projectTypes", "id");
```
The schema is used by:

- dataset validation
- JSON mapping
- GraphQL selection building

This reduces duplicated field lists and lowers the risk of forgetting a field in one layer.

## Schema-Driven Validation

Dataset validation is driven by `PortalSchema`.

The validator checks:

- required fields
- positive keys
- unique values
- references between tables

Examples:
```c++
    users.id is unique
    users.username is unique
    users.role references roles.code
    projects.ownerId references users.id
    projects.projectTypeId references projectTypes.id
```
## GraphQL Providers

GraphQL Providers implement the same Provider interfaces as memory providers.
```text
    PortalProjectProvider
            │
            ▼
    PortalGraphQLProjectProvider
            │
            ▼
    GraphQLClient
```
The Provider does not know whether the client talks to:

- a real HTTP GraphQL server
- `StaticGraphQLClient`
- `PortalDatasetGraphQLClient`

## Documents

Documents build GraphQL operations.

Examples:
```c++
    ProjectQueries::all()
    ProjectQueries::findById(id)
    ProjectMutations::create(project)
    UserQueries::all()
    RoleQueries::findByCode()
    ProjectTypeQueries::findById()
```
Documents should contain GraphQL operation structure only.

Field selections can be generated from `PortalSchema`:
```c++
    PortalGraphQLSelectionBuilder::from(
        PortalSchema::projects()
    );
```
## Mappers

Mappers convert between GraphQL data and application models.

Responsibilities:

- GraphQL response → Model
- Model → GraphQL variables

For model-to-JSON conversion, mappers can use:
```c++
    PortalSchemaMapper::toJson(
        PortalSchema::projects(),
        project
    );
```
This avoids repeating field assignments manually.

## Dataset GraphQL Client

`PortalDatasetGraphQLClient` is an application-level in-memory GraphQL client.

It interprets named GraphQL operations and reads/writes `PortalDataset`.
```text
    GraphQL operation
            │
            ▼
    PortalDatasetGraphQLClient
            │
            ▼
    PortalDataset
```
This allows Portal Demo and application tests to run without:

- HTTP server
- browser
- database
- Docker

## Reference Data

Reference entities are normal models.

Current examples:

    PortalRole
    PortalProjectType

They have:

- model
- schema
- provider
- GraphQL queries
- dataset-backed GraphQL support

References are declared in schema:
```c++
    users.role -> roles.code
    projects.projectTypeId -> projectTypes.id
```
## Application Tests

`PortalApplicationTestHost` allows testing real application flows without browser or HTTP infrastructure.

Example:
```c++
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    app.post<PortalCreateProjectAction>({
        {"title", "Application Test Project"},
        {"status", "active"}
    });

    EXPECT_EQ(app.projectCount(), 3);
```
This tests:
```text
    Action
        ↓
    Validation / Auth
        ↓
    Provider
        ↓
    GraphQL Client
        ↓
    Dataset
```
## Switching Data Sources

Changing the data source should require only DI registration changes.

Example:
```c++
    services.addFactory<PortalProjectProvider>(
        ServiceLifetime::Singleton,
        [graphQLClient] {
            return std::make_shared<PortalGraphQLProjectProvider>(
                graphQLClient
            );
        }
    );
```
No changes should be required in:

- Pages
- Actions
- Templates
- business logic

## Design Principles

- Pages and Actions depend on Provider interfaces.
- Providers exchange models, not parameter lists.
- Concrete data access is replaceable through DI.
- Dataset is the canonical demo/test state.
- Schema is the source of truth for fields, keys, uniqueness, required values, and references.
- Validation and mapping should reuse schema metadata.
- GraphQL field selections should be generated from schema where possible.
- Reference data should use the same architecture as normal models.
- Test infrastructure should reuse the same application building blocks as runtime code.
- New abstractions should be promoted only after they prove useful in Portal Demo.
## Boundary Lifetime

Provider interfaces describe application behavior, not object lifetime. Their concrete lifetime is chosen through DI. A singleton provider that owns mutable state must provide its own synchronization; request-scoped Pages and Actions do not make referenced singleton providers thread-safe.

Pages and Actions should resolve providers through their current context rather than constructing concrete providers directly. This keeps transport selection in application startup and keeps feature logic testable.
