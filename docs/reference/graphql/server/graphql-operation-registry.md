# `GraphQLOperationRegistry`

**Namespace:** `drogular`  
**Header:** `<drogular/graphql_operation_registry.hpp>`  
**Kind:** Class

## Purpose

`GraphQLOperationRegistry` maps named query and mutation operations to C++ handlers.

## Handler Types

```cpp
using Handler = std::function<GraphQLResponse(
    const GraphQLVariables&,
    const GraphQLExecutionContext&
)>;

using LegacyHandler = std::function<GraphQLResponse(
    const GraphQLVariables&
)>;
```

Legacy handlers are wrapped so they can coexist with context-aware handlers.

## Registration

```cpp
void registerQuery(std::string name, Handler handler);
void registerQuery(std::string name, LegacyHandler handler);
void registerMutation(std::string name, Handler handler);
void registerMutation(std::string name, LegacyHandler handler);
```

Registration uses `insert_or_assign()`. Registering the same name again replaces the previous handler for that operation kind.

Queries and mutations use separate maps, so the same name can exist once in each map.

## Execution

```cpp
GraphQLResponse executeQuery(... ) const;
GraphQLResponse executeMutation(... ) const;
```

Unknown operation names return an empty successful data object:

```json
{ "data": {} }
```

They do not produce a GraphQL `errors` entry or throw an exception.

The registry performs name-based dispatch only. It does not parse a schema or enforce requested field selections.

## Thread Safety

The registry provides no internal synchronization. Complete registration before concurrent execution unless external synchronization is provided.

## See Also

- [`GraphQLServer`](graphql-server.md)
- [`GraphQLExecutionContext`](graphql-execution-context.md)
