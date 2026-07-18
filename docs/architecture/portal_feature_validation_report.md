# Portal Demo Feature Validation

## Scope

Compared the current `projects` and `users` features after the feature-oriented layout refactoring.

Reviewed:

- provider interfaces;
- memory providers;
- GraphQL providers and mappers;
- create/update actions;
- list/edit pages;
- data models;
- query, sorting and pagination support;
- feature dependencies.

## Result

The feature structure is validated, but shared CRUD infrastructure is not yet validated.

Both features follow the same architectural direction:

```text
Feature
├── data
├── providers
├── graphql
├── actions
├── pages
└── ui
```

However, they are currently at different maturity levels.

`projects` contains the complete list pipeline:

```text
HTTP request
→ PortalProjectQueryParser
→ PortalProjectQuery
→ PortalProjectProvider::search
→ PortalPage<PortalProject>
→ PortalProjectQuerySerializer
→ template
```

`users` still uses the earlier list pipeline:

```text
HTTP request
→ PortalUserProvider::all
→ vector<PortalUser>
→ template
```

Because of this asymmetry, extracting generic Query, Parser, Serializer, list-page, provider or pagination infrastructure now would encode assumptions based primarily on Projects.

## Confirmed similarities

### 1. Feature layout

The same directory vocabulary works naturally for both domains:

- `data`;
- `providers`;
- `graphql`;
- `actions`;
- `pages`;
- `ui` when needed.

This structure should be retained.

### 2. Provider role

Both provider interfaces define the application-facing boundary and hide memory/GraphQL implementations.

Common conceptual operations exist:

- `all`;
- `findById`;
- `create`;
- `update`.

This is not enough evidence for a generic provider interface. The signatures and domain operations already differ:

- Projects require `ownerId`, removal, owner lookup and search.
- Users require credentials lookup, username lookup and duplicate checking.

### 3. Action flow

Create and update actions share a broad sequence:

```text
authorize
→ read form
→ validate
→ build input DTO
→ call provider
→ redirect
```

The actual authorization rules, validation rules, form fields, duplicate checks, provider calls and redirects are domain-specific. A generic CRUD action would currently remove clarity rather than duplication.

### 4. GraphQL provider flow

Both GraphQL providers repeat the mechanical sequence:

```text
build document and variables
→ client.execute
→ extract field
→ mapper
```

This is a possible future Drogular-level improvement, but the current repetition is small and explicit. It should not be abstracted before another feature confirms the same need.

### 5. Edit pages

Both edit pages perform:

```text
authorize
→ parse route id
→ load entity
→ expose not-found state
→ build schema-driven form context
```

The form structure and dependencies differ substantially. Keep them concrete.

## Rejected abstractions for now

Do not introduce yet:

- `BaseQuery`;
- `CrudProvider<T>`;
- `GenericCrudAction`;
- `GenericListPage`;
- `FeatureBase`;
- universal GraphQL provider;
- universal form/view-model mapper.

None of these has been validated by two equivalent implementations.

## Actual architectural gap

Users must first reach parity with the Projects list pipeline.

Recommended next PR:

1. Add `PortalUserQuery`.
2. Add `PortalUserSort` and sort direction.
3. Add `PortalUserQueryParser`.
4. Add `PortalUserQuerySerializer`.
5. Add `PortalUserQueryViewModel` if the template benefits from it.
6. Add `PortalUserProvider::search(const PortalUserQuery&)` returning `PortalPage<PortalUser>`.
7. Implement search, role filtering, deterministic sorting and pagination in the memory provider.
8. Add GraphQL query/page mapping for users.
9. Update `PortalUsersPage` to use the Query pipeline.
10. Preserve filters and sorting in pagination links.
11. Add parser, serializer, provider and page tests.

Suggested initial query fields:

```cpp
struct PortalUserQuery {
    std::optional<std::string> search;
    std::optional<std::string> role;
    std::vector<PortalUserSort> sorting;
    int page = 1;
    int pageSize = 10;
};
```

Suggested sortable fields:

- `username`;
- `role`;
- `id`.

Default ordering:

```text
username ascending
id ascending as deterministic tie-breaker
```

## Decision

The feature-oriented architecture has passed its first validation.

The generic infrastructure phase has not started yet. The next correct step is not abstraction, but a second complete implementation of the Query/Page pattern for Users. Only after that PR should Projects and Users be compared again for extraction candidates.
