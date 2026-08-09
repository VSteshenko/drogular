# GraphQL

Drogular provides a small GraphQL stack for building operation documents, transporting requests, reading responses, and dispatching named operations inside the same process.

The API is split into four areas so that applications can depend only on the layer they need.

```text
GraphQL document
      │
      ▼
GraphQLClient
   │       │
   │       └───────────────┐
   ▼                       ▼
HTTP endpoint       In-process GraphQLServer
   │                       │
   └──────────┬────────────┘
              ▼
       GraphQLResponse
              │
              ├── application mapper/provider
              └── GraphQLResult / RenderContext
```

## Client

- [`GraphQLClient`](client/graphql-client.md) — transport abstraction for queries, mutations, and raw requests.
- [`HttpGraphQLClient`](client/http-graphql-client.md) — synchronous HTTP transport using Drogon.
- [`InProcessGraphQLClient`](client/in-process-graphql-client.md) — dispatches named operations directly to a `GraphQLServer`.
- [`StaticGraphQLClient`](client/static-graphql-client.md) — deterministic test client that records requests.

## Requests and Responses

- [`GraphQLRequest`](requests/graphql-request.md) — GraphQL request body.
- [`GraphQLResponse`](requests/graphql-response.md) — access to `data`, `errors`, `extensions`, and raw JSON.
- [`GraphQLVariables`](requests/graphql-variables.md) — typed convenience builder for variables.

## Document Builder

- [`gql::Query`](documents/query.md)
- [`gql::Mutation`](documents/mutation.md)
- [`gql::Selection`](documents/selection.md)
- [`gql::Fragment`](documents/fragment.md)
- [`gql::Value`](documents/value.md)
- [`gql::ValidationResult`](documents/validation-result.md)

The document builder serializes GraphQL text. Its validation is intentionally limited and does not replace GraphQL schema validation.

## Server

- [`GraphQLServer`](server/graphql-server.md) — owns operation groups and dispatches named operations.
- [`GraphQLOperationRegistry`](server/graphql-operation-registry.md) — maps query/mutation names to handlers.
- [`GraphQLExecutionContext`](server/graphql-execution-context.md) — request-scoped string metadata passed to server handlers.

## Rendering Integration

- [`GraphQLResult`](../rendering/graphql-result.md) stores typed GraphQL-derived values used by `RenderContext`.
- [`RenderContext`](../rendering/render-context.md) can execute GraphQL operations and merge response data into its `GraphQLResult`.

## Cookbook

See [GraphQL](../../cookbook/graphql.md) for the recommended application-level organization of documents, providers, variables, and mappers.
