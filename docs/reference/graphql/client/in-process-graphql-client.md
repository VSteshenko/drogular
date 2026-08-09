# `InProcessGraphQLClient`

**Namespace:** `drogular`  
**Header:** `<drogular/in_process_graphql_client.hpp>`  
**Kind:** Final class

## Purpose

`InProcessGraphQLClient` preserves the `GraphQLClient` abstraction while dispatching operations directly to a `GraphQLServer` without HTTP.

PortalDemo uses this mode so application providers can use the same client interface while the demo server remains in the same process.

## Construction

```cpp
using ContextFactory = std::function<GraphQLExecutionContext()>;

explicit InProcessGraphQLClient(
    std::shared_ptr<GraphQLServer> server,
    ContextFactory contextFactory = {}
);
```

A null server throws `std::invalid_argument`.

The optional context factory is invoked for every execution.

## Query and Mutation Execution

The builder overloads dispatch by operation name:

```cpp
server_->executeQuery(query.name(), variables, createContext());
server_->executeMutation(mutation.name(), variables, createContext());
```

The current implementation does **not** interpret the selection set, fragments, aliases, or field arguments when executing in process. Server handlers are selected only by the named operation and receive the provided variables/context.

## Raw Requests

`executeRequest()` parses only the operation header needed to determine:

- query vs mutation;
- operation name.

Raw requests must use a named `query` or `mutation`.

Anonymous operations such as `{ viewer { id } }` throw `GraphQLClientError`. `subscription` is unsupported and also throws.

The parser is not a complete GraphQL parser; after the operation name it only validates enough structure to dispatch the request.

## Example

```cpp
auto server = createPortalGraphQLServer(dataset);

auto client = std::make_shared<drogular::InProcessGraphQLClient>(
    server
);
```

## See Also

- [`GraphQLClient`](graphql-client.md)
- [`GraphQLServer`](../server/graphql-server.md)
- [`GraphQLExecutionContext`](../server/graphql-execution-context.md)
