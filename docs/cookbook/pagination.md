# Pagination

## Problem

**Need to display a large collection while preserving search or filter parameters between pages?**

This guide shows how to parse pagination parameters defensively, page an application result, and generate navigation URLs without losing the active query.

---

## Recommended Solution

Keep HTTP parsing separate from pagination logic:

1. parse query parameters into an application query object with `RequestParameters`;
2. let the store/repository/provider return a `PagedResult<T>`;
3. normalize the requested page from the returned result;
4. build a `PaginationModel` whose URLs preserve the active filters.

This keeps pages focused on presentation and prevents raw request strings from leaking into data access code.

---

## Parse Request Parameters

`RequestParameters` performs strict integer parsing and provides helpers for positive and bounded values.

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

Invalid, missing, zero, or negative page values fall back to the chosen default instead of being passed further into the application.

---

## Return a Paged Result

For an in-memory collection, `paginate()` produces `PagedResult<T>` and clamps the requested page to the available range.

```cpp
return drogular::paginate(
    filtered,
    query.page,
    query.pageSize
);
```

For a database or remote provider, apply the same contract at the provider boundary: return the page items together with the normalized page number, page size, total item count, and total page count.

The page should consume that contract rather than recalculate pagination metadata itself.

---

## Preserve Filters in Navigation

After the provider returns the normalized page, update the query before generating links.

```cpp
const auto result = store->find(query);
query.page = result.page;
```

Then build page URLs from a copy of the current query.

```cpp
const auto pageUrl = [&query](int page) {
    auto pageQuery = query;
    pageQuery.page = page;

    return std::string("/") +
        TodoQuerySerializer::toQueryString(pageQuery);
};
```

This preserves search text, page size, sorting, or other filters while changing only the page number.

---

## Build the Pagination Model

Use `makePaginationModel()` to create the JSON model consumed by a component or template.

```cpp
context.setJson(
    "pagination",
    drogular::makePaginationModel(
        result.page,
        result.totalPages,
        pageUrl
    )
);
```

A reusable component can then render previous/next links and page metadata without knowing how the application's query is structured.

```text
HTTP query string
      │
      ▼
RequestParameters
      │
      ▼
Application Query
      │
      ▼
Store / Repository / Provider
      │
      ▼
PagedResult<T>
      │
      ▼
PaginationModel
      │
      ▼
Pagination Component
```

---

## Best Practices

- Parse request parameters once at the HTTP boundary.
- Use `positiveIntegerOr()` or `boundedPositiveIntegerOr()` for user-controlled page sizes and page numbers.
- Keep pagination metadata in `PagedResult<T>` instead of recomputing it in the template.
- Normalize the query to the returned page before generating navigation links.
- Preserve active filters when generating page URLs.
- Keep URL serialization in a query serializer/helper rather than concatenating parameters throughout the page.
- Apply limits to user-controlled page sizes.

---

## See Also

### API Reference

- [`RequestParameters`](../reference/pagination-and-request-parameters/request-parameters.md)
- [`PagedResult<T>`](../reference/pagination-and-request-parameters/paged-result.md)
- [`paginate()`](../reference/pagination-and-request-parameters/paginate.md)
- [`PaginationModel`](../reference/pagination-and-request-parameters/pagination-model.md) — also documents `makePaginationModel()`.
