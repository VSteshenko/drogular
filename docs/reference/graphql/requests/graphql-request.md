# `GraphQLRequest`

**Namespace:** `drogular`  
**Header:** `<drogular/graphql_request.hpp>`  
**Kind:** Class

## Purpose

`GraphQLRequest` represents the JSON body sent to a GraphQL endpoint.

## Public API

```cpp
explicit GraphQLRequest(std::string query = {});

void setQuery(std::string query);
const std::string& query() const;

void setVariable(std::string name, Json::Value value);
const Json::Value& variables() const;

GraphQLRequest& query(std::string query);
GraphQLRequest& variable(std::string name, Json::Value value);
GraphQLRequest& variables(const GraphQLVariables& variables);

Json::Value toJson() const;
```

The fluent and non-fluent setters have the same effect.

## JSON Shape

`toJson()` always emits both members:

```json
{
  "query": "...",
  "variables": { }
}
```

A directly constructed request starts with an empty object for `variables`.

When `variables(const GraphQLVariables&)` is called with a newly constructed, untouched `GraphQLVariables`, the copied JSON value is currently `null` rather than an empty object.

## See Also

- [`GraphQLVariables`](graphql-variables.md)
- [`GraphQLClient`](../client/graphql-client.md)
