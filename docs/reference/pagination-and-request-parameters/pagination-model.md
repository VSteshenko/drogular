# `PaginationModel`

**Namespace:** `drogular`  
**Header:** `<drogular/pagination_model.hpp>`  
**Kind:** Struct

## Purpose

`PaginationModel` contains the navigation state commonly required by a server-rendered pagination component: visibility, previous/next links, and numbered page links.

The model contains URLs rather than application query state. URL generation is delegated to the application through `makePaginationModel()`.

---

## Synopsis

```cpp
struct PaginationModel {
    bool visible{false};
    bool hasPrevious{false};
    bool hasNext{false};
    std::string previousUrl;
    std::string nextUrl;
    std::vector<PaginationLink> pages;
};
```

---

## Members

### `visible`

True when `totalPages > 1` in the model produced by `makePaginationModel()`.

### `hasPrevious`

Indicates whether a previous page exists.

### `hasNext`

Indicates whether a next page exists.

### `previousUrl`

URL for the previous page when `hasPrevious` is true. Otherwise empty in a model produced by `makePaginationModel()`.

### `nextUrl`

URL for the next page when `hasNext` is true. Otherwise empty in a model produced by `makePaginationModel()`.

### `pages`

One [`PaginationLink`](pagination-link.md) for every page from 1 through `totalPages`.

The current implementation does not collapse large page ranges or insert ellipses.

---

## `makePaginationModel()`

```cpp
template <typename PageUrlBuilder>
PaginationModel makePaginationModel(
    int currentPage,
    int totalPages,
    PageUrlBuilder&& pageUrl
);
```

Builds a navigation model and delegates URL creation to `pageUrl(pageNumber)`.

The function normalizes inputs before generating links:

```text
totalPages = max(1, totalPages)
currentPage = clamp(currentPage, 1, totalPages)
```

The URL callback is invoked:

- once for the previous page when available;
- once for the next page when available;
- once for every numbered page.

For three pages and a current page of 2, the callback is therefore invoked five times. Keep the callback inexpensive and free of side effects.

`visible` is false for a single-page result, but `pages` still contains one link for page 1.

---

## JSON Conversion

```cpp
Json::Value toJson(
    const PaginationModel& model
);
```

Produces:

```text
visible
hasPrevious
hasNext
previousUrl
nextUrl
pages
```

`pages` is an array of JSON-converted `PaginationLink` values.

The model can therefore be passed directly through `RenderContext::setJson()`:

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

---

## Example

TodoPWA preserves the current search and page-size state while changing only the page number:

```cpp
const auto pageUrl = [&query](int page) {
    auto pageQuery = query;
    pageQuery.page = page;

    return std::string("/") +
        TodoQuerySerializer::toQueryString(pageQuery);
};

const auto pagination = drogular::makePaginationModel(
    result.page,
    result.totalPages,
    pageUrl
);
```

PortalDemo uses the same pattern for users, projects, and departments.

---

## Related Types

- [`PaginationLink`](pagination-link.md)
- [`PagedResult<T>`](paged-result.md)
- [`QueryStringBuilder`](query-string-builder.md)
- [`RenderContext`](../rendering/render-context.md)
