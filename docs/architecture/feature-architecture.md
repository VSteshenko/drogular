# Portal Demo Feature Architecture

Portal Demo validates Drogular application architecture through independent domain features. Projects, Users, and Departments now all implement the complete query/list pipeline, while Roles, Project Types, Department Members, Authentication, Localization, Dashboard, and Diagnostics exercise narrower responsibilities.

## Feature ownership

A mature searchable feature owns its domain-specific pieces:

```text
feature/
├── data
├── providers
├── graphql
├── actions
├── pages
└── ui
```

For Projects, Users, and Departments this includes:

- domain models and create/update DTOs;
- provider contract;
- memory provider;
- GraphQL provider, documents, and mapping;
- actions;
- pages;
- feature query and sort model;
- request query parser;
- query-string serializer;
- focused tests.

The directory vocabulary is shared. Domain behavior is not forced into a generic base class.

## Validated list flow

All three mature list features now follow the same architectural direction:

```text
HTTP request
  -> FeatureQueryParser
  -> FeatureQuery
  -> FeatureProvider::search(query)
  -> PortalPage<FeatureModel>
  -> Page view data
  -> template
  -> FeatureQuerySerializer for links
```

The query object is the boundary between HTTP request syntax and provider behavior. Providers therefore do not depend on `HttpRequest`, query strings, or templates.

## Responsibilities

### Pages

Pages coordinate request-specific presentation work:

- resolve application services;
- parse route/query input into feature-level models;
- invoke providers;
- build render-context values and pagination links;
- stop before protected data is populated when authorization fails.

Pages do not implement transport-specific data access.

### Actions

Actions coordinate commands:

```text
authorize
  -> read and convert input
  -> validate
  -> build domain input
  -> invoke provider
  -> return ActionResult
```

Validation and authorization rules remain feature-specific even when this broad sequence repeats.

### Providers

Providers form the application-facing data boundary. They exchange domain models, input DTOs, query objects, and `PortalPage<T>` rather than HTTP or GraphQL types.

Memory and GraphQL implementations satisfy the same contract, allowing tests and runtime to switch infrastructure through DI registration.

### UI parsing and serialization

Parsers translate external query parameters into feature query objects. Serializers translate query state back into links. Keeping both directions explicit prevents providers and templates from owning URL semantics.

## What three features validate

Projects, Users, and Departments provide enough evidence to treat the following as established **application architecture**:

- feature-owned query models;
- parser/query/provider separation;
- provider-level pagination results;
- deterministic sorting;
- query serialization that preserves navigation state;
- memory and GraphQL implementations behind the same provider contract.

They do **not** yet prove that Drogular needs generic CRUD or feature base classes.

The three domains already differ meaningfully in relationships, filters, authorization, commands, view data, and provider operations. A universal abstraction would currently hide those differences for relatively little mechanical reduction.

## Extraction rule

Infrastructure moves from Portal Demo into Drogular only when all of the following are true:

1. the same need appears in multiple independent features;
2. the API shape repeats, not only the folder name;
3. the abstraction removes mechanical work without absorbing domain policy;
4. the resulting API can be explained independently from Portal Demo.

This rule is why generic `CrudProvider<T>`, `GenericCrudAction`, `FeatureBase`, and generic list-page classes are still intentionally absent.
