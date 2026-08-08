# `RequestParameters`

**Namespace:** `drogular`  
**Header:** `<drogular/request_parameters.hpp>`  
**Kind:** Utility class

## Purpose

`RequestParameters` wraps a Drogon `HttpRequestPtr` and provides small helpers for reading strings and integers with validation and fallbacks.

It is useful when a page needs to convert HTTP query parameters into an application-specific query object before calling a repository or provider.

---

## Construction

```cpp
explicit RequestParameters(
    drogon::HttpRequestPtr request
);
```

The request pointer is stored by value.

A null request is allowed. Read operations then behave as if all parameters were missing.

```cpp
const drogular::RequestParameters parameters(request);
```

---

## Public API

### `value()`

```cpp
std::string value(
    std::string_view name
) const;
```

Returns `HttpRequest::getParameter()` for the requested name.

Returns an empty string when:

- the request pointer is null;
- the parameter is missing;
- Drogon returns an empty value.

The method therefore does not distinguish a missing parameter from a present parameter whose value is empty.

### `optionalString()`

```cpp
std::optional<std::string> optionalString(
    std::string_view name
) const;
```

Returns the parameter value when it is non-empty.

Returns `std::nullopt` for missing or empty values.

### `integer()`

```cpp
std::optional<int> integer(
    std::string_view name
) const;
```

Parses the entire parameter value as a base-10 `int` using `std::from_chars()`.

Returns `std::nullopt` when the value is missing, empty, malformed, or contains unparsed trailing characters.

Negative integers are valid.

```cpp
// ?page=-2
parameters.integer("page"); // -2
```

### `positiveInteger()`

```cpp
std::optional<int> positiveInteger(
    std::string_view name
) const;
```

Returns a parsed integer only when it is greater than zero.

Zero, negative, missing, and malformed values return `std::nullopt`.

### `integerOr()`

```cpp
int integerOr(
    std::string_view name,
    int fallback
) const;
```

Returns `integer(name)` or `fallback` when parsing fails.

The fallback is not validated.

### `positiveIntegerOr()`

```cpp
int positiveIntegerOr(
    std::string_view name,
    int fallback
) const;
```

Returns a positive parsed integer or `fallback`.

The fallback itself is not required to be positive.

### `boundedPositiveIntegerOr()`

```cpp
int boundedPositiveIntegerOr(
    std::string_view name,
    int fallback,
    int maximum
) const;
```

Returns the positive parsed value or `fallback`, then applies:

```cpp
std::min(value, maximum)
```

The method imposes only an upper bound. It does not validate that `fallback` or `maximum` are positive.

---

## Behavior

`RequestParameters` performs parsing only. It does not:

- modify the request;
- URL-decode values itself beyond whatever Drogon exposes through `getParameter()`;
- report validation errors;
- normalize application-specific enum or sort values.

Application-specific parsing remains in the caller.

PortalDemo, for example, validates allowed sort fields after reading the raw string.

---

## Example

TodoPWA converts request parameters into `TodoQuery`:

```cpp
class TodoQueryParser {
public:
    static TodoQuery fromRequest(
        const drogon::HttpRequestPtr& request
    ) {
        const drogular::RequestParameters parameters(request);

        TodoQuery query;
        query.search = parameters.value("search");
        query.page = parameters.positiveIntegerOr("page", 1);
        query.pageSize = parameters.boundedPositiveIntegerOr(
            "pageSize",
            5,
            20
        );

        return query;
    }
};
```

This keeps HTTP parsing separate from the page and from the store that performs the search.

---

## Related Types

- [`QueryStringBuilder`](query-string-builder.md)
- [`PagedResult<T>`](paged-result.md)
- [`RenderContext`](../rendering/render-context.md)
