# `PagedResult<T>`

**Namespace:** `drogular`  
**Header:** `<drogular/paged_result.hpp>`  
**Kind:** Struct template

## Purpose

`PagedResult<T>` represents one page of values together with the metadata required to render or serialize pagination state.

Repositories and providers can return this type regardless of whether their data comes from an in-memory vector, GraphQL, or another backend.

---

## Synopsis

```cpp
template <typename T>
struct PagedResult {
    std::vector<T> items;

    int page{1};
    int pageSize{10};
    int totalItems{0};
    int totalPages{1};

    bool hasPrevious() const noexcept;
    bool hasNext() const noexcept;
};
```

---

## Template Parameters

### `T`

The element type stored in `items`.

---

## Members

### `items`

```cpp
std::vector<T> items;
```

Values belonging to the current page.

### `page`

```cpp
int page{1};
```

Current 1-based page number.

### `pageSize`

```cpp
int pageSize{10};
```

Requested or effective number of items per page.

### `totalItems`

```cpp
int totalItems{0};
```

Total number of matching values across all pages.

### `totalPages`

```cpp
int totalPages{1};
```

Total number of pages.

The struct itself does not enforce consistency between these fields. Producers are responsible for filling valid metadata.

---

## Public API

### `hasPrevious()`

```cpp
bool hasPrevious() const noexcept;
```

Returns `page > 1`.

### `hasNext()`

```cpp
bool hasNext() const noexcept;
```

Returns `page < totalPages`.

Neither helper validates the metadata before evaluating it.

---

## Example

A provider can expose pagination without leaking its storage implementation:

```cpp
virtual drogular::PagedResult<PortalUser> search(
    const PortalUserQuery& query
) = 0;
```

For in-memory collections, [`paginate()`](paginate.md) builds the result automatically.

GraphQL-backed PortalDemo providers construct the same `PagedResult<T>` from remote response data, allowing pages to consume both implementations through the same provider interface.

---

## Related Types

- [`paginate()`](paginate.md)
- [`PaginationModel`](pagination-model.md)
- [`RequestParameters`](request-parameters.md)
