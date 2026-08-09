# `HttpGraphQLClient`

**Namespace:** `drogular`  
**Header:** `<drogular/graphql_client.hpp>`  
**Kind:** Final class

## Purpose

`HttpGraphQLClient` sends GraphQL requests to an HTTP endpoint through Drogon's `HttpClient`.

## Construction

```cpp
HttpGraphQLClient(
    std::string host,
    std::uint16_t port,
    std::string path = "/graphql"
);
```

The endpoint base is constructed as `http://<host>:<port>`. The current API does not expose HTTPS/TLS configuration.

## Behavior

`executeRequest()`:

1. creates a Drogon HTTP client;
2. sends a `POST` request containing `GraphQLRequest::toJson()`;
3. waits synchronously for the asynchronous Drogon callback through `std::promise` / `std::future`;
4. requires an HTTP 2xx status;
5. requires a JSON response body;
6. returns `GraphQLResponse`.

It throws `GraphQLClientError` for request failure, non-2xx status, or invalid JSON.

### GraphQL errors

```cpp
client.execute(query, variables);
client.execute(mutation, variables);
```

These typed overloads throw `GraphQLClientError("GraphQL response contains errors")` when `GraphQLResponse::hasErrors()` is true.

```cpp
client.executeRequest(request);
```

`executeRequest()` itself does **not** throw merely because the returned JSON contains a GraphQL `errors` array. Callers that use raw requests must inspect the response explicitly.

## Example

```cpp
auto client = std::make_shared<drogular::HttpGraphQLClient>(
    "127.0.0.1",
    8080,
    "/graphql"
);
```

## Threading

Execution blocks the calling thread until Drogon's request callback completes.

## See Also

- [`GraphQLClient`](graphql-client.md)
- [`GraphQLRequest`](../requests/graphql-request.md)
- [`GraphQLResponse`](../requests/graphql-response.md)
