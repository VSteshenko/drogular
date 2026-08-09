# `GraphQLVariables`

**Namespace:** `drogular`  
**Header:** `<drogular/graphql_variables.hpp>`  
**Kind:** Class

## Purpose

`GraphQLVariables` builds the variables JSON passed alongside a GraphQL operation.

## Public API

```cpp
GraphQLVariables& set(const std::string& name, const std::string& value);
GraphQLVariables& set(const std::string& name, int value);
GraphQLVariables& set(const std::string& name, bool value);
GraphQLVariables& set(const std::string& name, const Json::Value& value);
GraphQLVariables& set(const std::string& name, const char* value);

const Json::Value& json() const;
```

Repeated `set()` calls for the same name replace the previous JSON member.

A null `const char*` is stored as an empty string.

A newly constructed object contains a default `Json::Value`, which is `null` until the first variable is assigned.

## Example

```cpp
drogular::GraphQLVariables variables;
variables
    .set("page", 2)
    .set("search", "vadim")
    .set("active", true);
```

## See Also

- [`GraphQLRequest`](graphql-request.md)
- [`gql::Query`](../documents/query.md)
- [`gql::Mutation`](../documents/mutation.md)
