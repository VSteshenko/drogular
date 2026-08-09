# `gql::ValidationResult`

**Namespace:** `drogular::gql`  
**Header:** `<drogular/graphql.hpp>`  
**Kind:** Class

## Purpose

`gql::ValidationResult` stores errors produced by Drogular's lightweight GraphQL document-builder validation.

This type is separate from `drogular::ValidationResult` used by forms.

## Public API

```cpp
void addError(std::string error);
bool valid() const;
const std::vector<std::string>& errors() const;
```

`valid()` is true when the error vector is empty.

The validation performed by `gql::Query` and `gql::Mutation` is structural helper validation only. It does not validate documents against a GraphQL schema.

## See Also

- [`gql::Query`](query.md)
- [`gql::Mutation`](mutation.md)
- [`ValidationResult`](../../forms-and-validation/validation-result.md)
