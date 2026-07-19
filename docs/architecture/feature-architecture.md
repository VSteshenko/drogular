# Portal Demo feature architecture

Portal Demo validates Drogular application architecture through three independent features: Projects, Users, and Departments.

Each mature feature owns its data models, provider contract, provider implementations, actions, pages, query parsing, query serialization, and tests. External input is converted to a feature query before reaching a provider. Providers return domain models or `PortalPage<T>` and do not depend on HTTP or templates. Pages coordinate services and build view data; actions enforce authorization, validate input, and invoke providers.

Infrastructure is extracted into Drogular only after the same architectural need is demonstrated by multiple independent features. Similar folder names alone are not evidence. The implementation and extension points must also repeat naturally.

## Validated flow

```text
HTTP request
  -> FeatureQueryParser
  -> FeatureQuery
  -> FeatureProvider::search
  -> PortalPage<FeatureModel>
  -> Page view data
  -> template and FeatureQuerySerializer links
```

## Current validation set

- Projects: relational filters, project types, owners, details, deletion.
- Users: credentials, roles, authorization-sensitive editing.
- Departments: active state, manager relation, descriptive search.

The next architecture review should compare these three implementations before introducing generic query, CRUD, page, or provider bases.
