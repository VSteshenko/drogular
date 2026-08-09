# `gql::Mutation`

**Namespace:** `drogular::gql`  
**Header:** `<drogular/graphql.hpp>`  
**Kind:** Class

## Purpose

`Mutation` builds a named GraphQL mutation document.

## Construction

```cpp
Mutation mutation(std::string name);
```

## Public API

```cpp
const std::string& name() const noexcept;
Mutation& variable(std::string name, std::string type);
Mutation& select(Selection selection);
Mutation& select(std::string name, std::vector<Selection> children);
Mutation& fragment(Fragment fragment);
ValidationResult validate() const;
std::string toString() const;
```

## Validation

Current mutation validation is intentionally smaller than query validation. It checks only:

- at least one selection is present;
- variable names are not duplicated.

It currently does **not** check mutation name emptiness, variable type/name emptiness, selection structure, fragment usage, or schema compatibility.

`toString()` does not call `validate()` automatically.

## Example

```cpp
const auto mutation = drogular::gql::mutation("CreateTodo")
    .variable("title", "String!")
    .select(
        drogular::gql::field("createTodo")
            .arg("title", drogular::gql::variable("title"))
            .children({
                drogular::gql::field("id"),
                drogular::gql::field("title")
            })
    );
```

## See Also

- [`gql::Query`](query.md)
- [`gql::Selection`](selection.md)
- [`gql::ValidationResult`](validation-result.md)
