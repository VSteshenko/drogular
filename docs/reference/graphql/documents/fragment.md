# `gql::Fragment`

**Namespace:** `drogular::gql`  
**Header:** `<drogular/graphql.hpp>`  
**Kind:** Class

## Purpose

`Fragment` represents a named GraphQL fragment definition.

## Construction

```cpp
Fragment fragment(
    std::string name,
    std::string typeName,
    std::vector<Selection> selections
);
```

## Public API

```cpp
std::string toString() const;
const std::string& name() const;
const std::string& typeName() const;
const std::vector<Selection>& selections() const;
```

Fragments are attached to queries or mutations through `.fragment(...)` and referenced with `gql::spread(name)`.

Query validation detects undefined and unused fragments. Mutation validation currently does not perform equivalent fragment checks.

## See Also

- [`gql::Selection`](selection.md)
- [`gql::Query`](query.md)
- [`gql::Mutation`](mutation.md)
