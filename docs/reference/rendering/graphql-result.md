# `GraphQLResult`

**Namespace:** `drogular`  
**Header:** `<drogular/render_context.hpp>`  
**Kind:** Class

## Purpose

`GraphQLResult` stores typed GraphQL-derived values associated with a `RenderContext`.

It is an in-memory C++ value container, not the raw GraphQL wire response. [`GraphQLResponse::toResult()`](../graphql/requests/graphql-response.md) is one way to populate it from response JSON.

## Public API

```cpp
template <typename T>
void set(std::string key, T value);

template <typename T>
std::optional<T> get(const std::string& key) const;

template <typename T>
T require(const std::string& key) const;

void merge(GraphQLResult other);
bool contains(const std::string& key) const;
void clear();
Json::Value toJson() const;
```

Values are stored as `std::any`. `get<T>()` returns `std::nullopt` when the key is absent or the C++ type differs.

`require<T>()` throws `RenderContextError` in either case.

`merge()` replaces existing keys with incoming values.

## Rendering Integration

`RenderContext::executeGraphQL()` converts the returned `GraphQLResponse` to a `GraphQLResult` and merges it into the context's existing GraphQL result, preserving unrelated previous keys.

## JSON Conversion

`toJson()` converts stored values through Drogular's supported `std::any` → JSON conversion path. Values that are not supported by that conversion cannot be represented faithfully in template JSON output.

## See Also

- [`RenderContext`](render-context.md)
- [`GraphQLResponse`](../graphql/requests/graphql-response.md)
- [`GraphQL Reference`](../graphql/README.md)
