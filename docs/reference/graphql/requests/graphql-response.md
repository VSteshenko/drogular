# `GraphQLResponse`

**Namespace:** `drogular`  
**Header:** `<drogular/graphql_response.hpp>`  
**Kind:** Class

## Purpose

`GraphQLResponse` wraps a GraphQL JSON response and provides access to `data`, `errors`, `extensions`, and the original JSON document.

## Public API

```cpp
explicit GraphQLResponse(Json::Value response);

const Json::Value& data() const;
bool hasData() const;
std::optional<Json::Value> field(const std::string& name) const;

const Json::Value& errors() const;
bool hasErrors() const;
std::vector<std::string> errorMessages() const;

bool hasExtensions() const;
const Json::Value& extensions() const;

const Json::Value& rawJson() const;
GraphQLResult toResult() const;
```

## Data

`hasData()` requires a present, non-null `data` member.

`field(name)` additionally requires `data` to be an object and returns a copy of the selected JSON field.

When `data`, `errors`, or `extensions` is absent, the corresponding accessor returns a shared empty `Json::Value` reference.

## Errors

`hasErrors()` is true only when `errors` is a non-empty JSON array.

`errorMessages()` collects:

- string entries directly;
- object entries whose `message` member is a string.

Other error shapes are ignored by this convenience method but remain available through `errors()` / `rawJson()`.

## `toResult()`

Converts top-level members of `data` into a [`GraphQLResult`](../../rendering/graphql-result.md) and always stores the complete original response under `"__json"`.

Primitive strings, integers, booleans, and doubles are stored as corresponding C++ values. Other JSON values are stored as `Json::Value`.

## See Also

- [`GraphQLRequest`](graphql-request.md)
- [`GraphQLResult`](../../rendering/graphql-result.md)
