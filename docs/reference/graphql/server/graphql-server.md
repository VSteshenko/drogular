# `GraphQLServer`

**Namespace:** `drogular`  
**Header:** `<drogular/graphql_server.hpp>`  
**Kind:** Class

## Purpose

`GraphQLServer` owns GraphQL operation groups and exposes name-based query/mutation execution for in-process use.

## Role in Drogular

```text
Operation group objects
        │ registerWith(...)
        ▼
GraphQLOperationRegistry
        ▲
        │ owned by
   GraphQLServer
        ▲
        │
InProcessGraphQLClient
        │
        ▼
Application providers
```

`GraphQLServer` is not an HTTP GraphQL endpoint and is not a full GraphQL execution engine. It is Drogular's in-process operation dispatcher.

## Adding Operation Groups

```cpp
template <typename TOperations, typename... TArguments>
GraphQLServer& add(TArguments&&... arguments);
```

`add()`:

1. constructs `TOperations` in a `std::shared_ptr`;
2. calls `operations->registerWith(registry_)`;
3. stores the operation object so handlers capturing `this` remain valid;
4. returns the server for chaining.

The operation type therefore needs a compatible `registerWith(GraphQLOperationRegistry&)` member.

PortalDemo uses this pattern:

```cpp
server
    ->add<ProjectGraphQLOperations>(dataset)
    .add<UserGraphQLOperations>(dataset)
    .add<DepartmentGraphQLOperations>(dataset);
```

## Execution

```cpp
GraphQLResponse executeQuery(
    const std::string& name,
    const GraphQLVariables& variables = {},
    const GraphQLExecutionContext& context = {}
) const;

GraphQLResponse executeMutation(... ) const;
```

Execution delegates directly to the registry.

Unknown names return `{ "data": {} }`.

## Limitations

The current server does not:

- parse a GraphQL schema;
- validate a document against a schema;
- execute field selection sets;
- apply aliases/fragments during server execution;
- expose an HTTP endpoint by itself.

The in-process path treats the operation **name** as the dispatch contract.

## Thread Safety

No internal synchronization is provided. Register operation groups during startup. Concurrent execution is only as safe as the registered handlers and their dependencies.

## See Also

- [`GraphQLOperationRegistry`](graphql-operation-registry.md)
- [`GraphQLExecutionContext`](graphql-execution-context.md)
- [`InProcessGraphQLClient`](../client/in-process-graphql-client.md)
