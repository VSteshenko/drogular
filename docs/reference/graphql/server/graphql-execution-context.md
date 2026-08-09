# `GraphQLExecutionContext`

**Namespace:** `drogular`  
**Header:** `<drogular/graphql_execution_context.hpp>`  
**Kind:** Class

## Purpose

`GraphQLExecutionContext` carries request-scoped string metadata from an in-process client to GraphQL operation handlers.

## Public API

```cpp
GraphQLExecutionContext& set(
    std::string name,
    std::string value
);

std::optional<std::string> value(
    const std::string& name
) const;
```

Repeated `set()` calls replace the existing value.

The current context stores only strings. It has no parent scope, typed values, or automatic integration with HTTP requests/sessions.

## Example

```cpp
drogular::GraphQLExecutionContext context;
context.set("actor", "admin");
```

## See Also

- [`InProcessGraphQLClient`](../client/in-process-graphql-client.md)
- [`GraphQLServer`](graphql-server.md)
