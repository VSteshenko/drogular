# `paginate()`

**Namespace:** `drogular`  
**Header:** `<drogular/pagination.hpp>`  
**Kind:** Function template

## Purpose

`paginate()` slices an in-memory `std::vector<T>` and returns a normalized [`PagedResult<T>`](paged-result.md).

It is intended for simple in-memory providers and examples. Database- or service-backed repositories normally perform pagination at the data source and construct `PagedResult<T>` directly.

---

## Synopsis

```cpp
template <typename T>
PagedResult<T> paginate(
    const std::vector<T>& values,
    int requestedPage,
    int requestedPageSize
);
```

---

## Parameters

### `values`

Complete source collection to paginate.

The selected page is copied into `PagedResult<T>::items`.

### `requestedPage`

Requested 1-based page number.

The value is clamped to the valid page range.

### `requestedPageSize`

Requested items per page.

Values below 1 are normalized to 1.

---

## Behavior

`paginate()` computes:

```text
pageSize   = max(1, requestedPageSize)
totalItems = values.size()
totalPages = max(1, ceil(totalItems / pageSize))
page       = clamp(requestedPage, 1, totalPages)
```

Important behavior:

- an empty collection still reports `totalPages == 1`;
- page numbers below 1 become 1;
- page numbers above the last page become `totalPages`;
- page size has a lower bound of 1 but no upper bound;
- items are copied from the source vector into the result;
- the function does not filter or sort values before slicing them.

For large or remote datasets, paginate at the data source instead of materializing all values and calling this helper.

---

## Example

TodoPWA filters its state first and then paginates the matching values:

```cpp
drogular::PagedResult<Todo> find(
    const TodoQuery& query
) const {
    std::vector<Todo> filtered;

    // Filtering omitted.

    return drogular::paginate(
        filtered,
        query.page,
        query.pageSize
    );
}
```

```cpp
const std::vector<int> values{1, 2, 3, 4, 5};
const auto result = drogular::paginate(values, 2, 2);

// result.items      == {3, 4}
// result.page       == 2
// result.totalPages == 3
```

---

## Related Types

- [`PagedResult<T>`](paged-result.md)
- [`PaginationModel`](pagination-model.md)
- [`makePaginationModel()`](pagination-model.md#makepaginationmodel)
