# Pagination and Request Parameters

Utilities for parsing list-related query parameters, preserving them in generated URLs, slicing in-memory collections into pages, and building template-friendly navigation models.

The types in this section are intentionally independent. Applications can use request parsing without pagination, return `PagedResult<T>` from repositories backed by any data source, and build pagination links with their own URL policy.

## Typical Flow

```text
HTTP Request
    │
    ▼
RequestParameters
    │
    ▼
Application Query
    │
    ▼
Repository / Provider
    │
    ▼
PagedResult<T>
    │
    ├───────────────┐
    ▼               ▼
Page data      makePaginationModel()
                    │
                    ▼
              PaginationModel
                    │
                    ▼
                 Template

Application Query
    │
    ▼
QueryStringBuilder
    │
    ▼
Stable navigation URL
```

TodoPWA and PortalDemo use this flow to keep search, sorting, page size, and page number stable while navigating between pages.

## Types and Functions

- [`RequestParameters`](request-parameters.md) — reads and validates query parameters from a Drogon request.
- [`QueryStringBuilder`](query-string-builder.md) — builds encoded query strings while conditionally omitting default values.
- [`PagedResult<T>`](paged-result.md) — carries one page of items and pagination metadata.
- [`paginate()`](paginate.md) — paginates an in-memory `std::vector<T>`.
- [`PaginationLink`](pagination-link.md) — describes one numbered pagination link.
- [`PaginationModel`](pagination-model.md) — describes previous/next and numbered navigation links.
- [`makePaginationModel()`](pagination-model.md#makepaginationmodel) — builds a `PaginationModel` from page metadata and a URL callback.

## Related Documentation

- [Routing API Reference](../routing/README.md)
- [`Url`](../routing/url.md)
- [`RenderContext`](../rendering/render-context.md)
- [Pagination Cookbook guide](../../cookbook/pagination.md)
