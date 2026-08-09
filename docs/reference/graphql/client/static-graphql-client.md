# `StaticGraphQLClient`

**Namespace:** `drogular`  
**Header:** `<drogular/static_graphql_client.hpp>`  
**Kind:** Class

## Purpose

`StaticGraphQLClient` is a deterministic test double that returns one predefined `data` object for every request and records executed requests.

## Construction

```cpp
explicit StaticGraphQLClient(Json::Value data);
```

The supplied value is wrapped as:

```json
{ "data": ... }
```

The constructor therefore defines response data, not a complete raw GraphQL response.

## Request Recording

```cpp
const std::vector<GraphQLRequest>& requests() const;
std::optional<GraphQLRequest> lastRequest() const;
void clearRequests();
std::size_t requestCount() const;
```

Queries and mutations are serialized into `GraphQLRequest` objects before being recorded.

## Behavior

Every request returns the same predefined response. The client does not inspect operation names, variables, or selection sets to produce different results.

It does not automatically simulate GraphQL errors or transport failures.

## Example

```cpp
Json::Value data(Json::objectValue);
data["viewer"]["name"] = "Vadim";

drogular::StaticGraphQLClient client(data);

client.execute(drogular::gql::query("Viewer"));

const auto request = client.lastRequest();
```

## Thread Safety

Request history is mutable and unsynchronized. Do not share one instance across concurrent tests without external synchronization.

## See Also

- [`GraphQLClient`](graphql-client.md)
- [`GraphQLRequest`](../requests/graphql-request.md)
