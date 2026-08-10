# `GraphQLClient`

**Namespace:** `drogular`  
**Header:** `<drogular/graphql_client.hpp>`  
**Kind:** Abstract class

## Purpose

`GraphQLClient` is the transport boundary used by application code to execute GraphQL queries, mutations, and complete request objects.

## Role in Drogular

```text
Application provider / RenderContext
              │
              ▼
        GraphQLClient
          │       │
          │       ├── HttpGraphQLClient
          │       ├── InProcessGraphQLClient
          │       └── StaticGraphQLClient
          ▼
     GraphQLResponse
```

Application code can depend on `GraphQLClient` while choosing HTTP, in-process dispatch, or a test implementation at configuration time.

## Public API

```cpp
virtual GraphQLResponse execute(
    const gql::Query& query,
    const GraphQLVariables& variables = {}
) = 0;

virtual GraphQLResponse execute(
    const gql::Mutation& mutation,
    const GraphQLVariables& variables = {}
) = 0;

virtual GraphQLResponse executeRequest(
    const GraphQLRequest& request
) = 0;
```

`execute()` accepts builder objects. `executeRequest()` accepts already serialized GraphQL text and variables.

## `GraphQLClientError`

`GraphQLClientError` derives from `DrogularError` and is used by concrete clients for transport or execution failures.

Concrete implementations do not all apply identical error policies. In particular, `HttpGraphQLClient::execute(query/mutation)` throws when a GraphQL response contains errors, while `executeRequest()` returns such a response unchanged.

## Thread Safety

The interface provides no synchronization guarantee. Thread safety depends on the concrete implementation.

## See Also

- [`HttpGraphQLClient`](http-graphql-client.md)
- [`InProcessGraphQLClient`](in-process-graphql-client.md)
- [`StaticGraphQLClient`](static-graphql-client.md)
- [`GraphQLRequest`](../requests/graphql-request.md)
- [`GraphQLResponse`](../requests/graphql-response.md)
