# `gql::Query`

**Namespace:** `drogular::gql`  
**Header:** `<drogular/graphql.hpp>`  
**Kind:** Class

## Purpose

`Query` builds a named GraphQL query document from variables, selections, and fragments.

## Construction

```cpp
Query query(std::string name);
```

## Public API

```cpp
const std::string& name() const noexcept;
Query& variable(std::string name, std::string type);
Query& select(Selection selection);
Query& select(std::string name, std::vector<Selection> children);
Query& fragment(Fragment fragment);
ValidationResult validate() const;
std::string toString() const;
```

The builder preserves insertion order for variables, selections, and fragments.

## Validation

Current query validation checks:

- non-empty query name;
- non-empty variable names and types;
- non-empty selection names;
- non-empty aliases when present;
- non-empty argument names;
- non-empty fragment names and type names;
- fragment spreads that reference undefined fragments;
- fragment definitions that are never used.

It does **not** validate against a schema, validate GraphQL types, check duplicate variables, or require at least one top-level selection.

`toString()` does not call `validate()` automatically.

## Example

```cpp
const auto query = drogular::gql::query("PortalUserByCredentials")
    .variable("username", "String!")
    .variable("password", "String!")
    .select(
        drogular::gql::field("userByCredentials")
            .arg("username", drogular::gql::variable("username"))
            .arg("password", drogular::gql::variable("password"))
            .children({
                drogular::gql::field("id"),
                drogular::gql::field("username")
            })
    );
```

## See Also

- [`gql::Selection`](selection.md)
- [`gql::Fragment`](fragment.md)
- [`GraphQLVariables`](../requests/graphql-variables.md)
