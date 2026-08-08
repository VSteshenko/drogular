# `QueryStringBuilder`

**Namespace:** `drogular`  
**Header:** `<drogular/query_string_builder.hpp>`  
**Kind:** Utility class

## Purpose

`QueryStringBuilder` builds a query-string suffix while percent-encoding values and optionally omitting empty or default parameters.

The builder returns strings beginning with `?` and is intended to be appended to an application path.

---

## Public API

### `add(name, value)`

```cpp
QueryStringBuilder& add(
    std::string_view name,
    std::string_view value
);

QueryStringBuilder& add(
    std::string_view name,
    int value
);
```

Adds a parameter unconditionally.

The first value starts with `?`; subsequent values use `&`.

String values are encoded through [`Url::encode()`](../routing/url.md).

```cpp
const auto query = drogular::QueryStringBuilder{}
    .add("search", "hello world")
    .add("page", 2)
    .build();

// ?search=hello%20world&page=2
```

An explicitly empty string is preserved:

```cpp
.add("search", "")
// ?search=
```

Parameter names are appended as supplied and are **not** percent-encoded by `QueryStringBuilder`.

### `addNonEmpty()`

```cpp
QueryStringBuilder& addNonEmpty(
    std::string_view name,
    std::string_view value
);
```

Adds the parameter only when `value` is not empty.

### `add(name, optional<string>)`

```cpp
template <typename T>
    requires std::same_as<T, std::string>
QueryStringBuilder& add(
    std::string_view name,
    const std::optional<T>& value
);
```

Adds only present, non-empty `std::optional<std::string>` values.

Both `std::nullopt` and `std::optional<std::string>{""}` are omitted.

### `addIf()`

```cpp
QueryStringBuilder& addIf(
    bool condition,
    std::string_view name,
    std::string_view value
);

QueryStringBuilder& addIf(
    bool condition,
    std::string_view name,
    int value
);
```

Adds the parameter only when `condition` is true.

Unlike `addNonEmpty()`, the string overload preserves an empty value when the condition is true.

### `build()`

```cpp
const std::string& build() const noexcept;
```

Returns a const reference to the accumulated query string.

The reference remains valid only while the builder remains alive and is not modified.

### `empty()`

```cpp
bool empty() const noexcept;
```

Returns true before any parameter has been added.

---

## Behavior

`QueryStringBuilder` builds only the query portion:

```text
?search=hello&page=2
```

It does not:

- prepend an application path;
- decode values;
- sort or deduplicate parameter names;
- replace an existing parameter;
- encode parameter names.

Adding the same name multiple times produces multiple query parameters in insertion order.

---

## Example

TodoPWA omits default pagination values so generated navigation URLs remain stable and compact:

```cpp
static std::string toQueryString(const TodoQuery& query) {
    return drogular::QueryStringBuilder{}
        .addNonEmpty("search", query.search)
        .addIf(query.pageSize != 5, "pageSize", query.pageSize)
        .addIf(query.page > 1, "page", query.page)
        .build();
}
```

PortalDemo uses the optional-string overload for filters:

```cpp
drogular::QueryStringBuilder builder;

builder.add("search", query.search);
builder.add("role", query.role);
```

---

## Related Types

- [`RequestParameters`](request-parameters.md)
- [`Url`](../routing/url.md)
- [`PaginationModel`](pagination-model.md)
