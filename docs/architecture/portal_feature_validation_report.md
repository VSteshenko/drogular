# Portal Demo Feature Validation

## Scope

This review compares the current Projects, Users, and Departments features after all three reached the query/search/pagination architecture.

Reviewed:

- feature layout;
- query and sorting models;
- request parsers and query serializers;
- provider contracts;
- memory providers;
- GraphQL providers, documents, and mappers;
- list/detail/edit pages where present;
- create/update/delete actions where present;
- tests around parsing, serialization, provider behavior, GraphQL mapping, and page flows.

## Result

The feature-oriented architecture is validated across three independent domains.

The earlier gap where Users still used `all()` and `vector<PortalUser>` is closed. Projects, Users, and Departments all now use a feature query and provider search boundary.

```text
Request
  -> FeatureQueryParser
  -> FeatureQuery
  -> FeatureProvider::search
  -> PortalPage<FeatureModel>
  -> Page / template
  -> FeatureQuerySerializer
```

This is enough evidence to document the pattern as recommended application architecture.

It is **not** enough evidence for generic CRUD framework infrastructure.

## Confirmed common architecture

### Feature query boundary

Projects, Users, and Departments each convert request parameters into a domain-specific query object before provider execution.

This boundary successfully isolates:

- HTTP parameter syntax;
- validation/defaulting of list controls;
- search/filter state;
- sorting state;
- pagination state.

Provider implementations receive application-level query objects rather than `HttpRequest`.

### Provider boundary

Each provider defines operations that make sense for its domain and hides whether the implementation is memory-backed or GraphQL-backed.

The shared concept is stable; a shared base interface is not required. Operation sets and domain rules differ enough that explicit provider interfaces remain clearer.

### Pagination result

Search operations return `PortalPage<T>`-style application data rather than template-specific pagination HTML. Pages convert that result into view data and serialized links.

This is a validated separation of concerns.

### GraphQL provider flow

GraphQL implementations repeatedly perform:

```text
feature query / input
  -> GraphQL document + variables
  -> GraphQLClient
  -> response field
  -> mapper
  -> domain model/page
```

This confirms the value of the provider boundary. The remaining mechanical repetition is still small enough that a universal GraphQL provider would reduce clarity.

### Action flow

Feature actions repeatedly follow:

```text
authorize
  -> parse/validate input
  -> construct domain input
  -> provider operation
  -> redirect/result
```

The sequence is common, but authorization, fields, validation, duplicate checks, side effects, and redirects are domain-specific. Generic CRUD actions remain rejected.

## Differences that should remain visible

Projects, Users, and Departments have different:

- filter sets;
- sorting options and defaults;
- relations;
- authorization rules;
- create/update semantics;
- detail/edit screens;
- provider operations;
- GraphQL documents and mapping.

A useful abstraction must preserve these differences rather than turning them into configuration metadata prematurely.

## Current decisions

### Keep as application-level patterns

- `FeatureQuery` + parser + serializer;
- domain-specific provider interfaces;
- memory/GraphQL provider pairs;
- concrete Page and Action classes;
- schema-driven Portal Demo mapping helpers;
- Portal pagination/select UI components.

### Keep out of Drogular for now

- `BaseQuery`;
- `CrudProvider<T>`;
- `GenericCrudAction`;
- `GenericListPage`;
- `FeatureBase`;
- universal GraphQL provider;
- universal domain/view-model mapper.

## Next validation target

The next architectural extraction should not be driven by adding a fourth CRUD feature merely to increase the count. Instead, compare mechanical repetition across the three existing mature features and extract only a helper whose responsibility is demonstrably transport- or domain-neutral.

Good candidates for observation include query-string mechanics, common provider-test harness utilities, and GraphQL response plumbing. They should remain application-level until an API can be expressed without knowing whether the feature is Projects, Users, or Departments.

## Decision

The feature architecture has moved from **experimental** to **validated application pattern**.

The generic CRUD infrastructure phase is still intentionally deferred.
