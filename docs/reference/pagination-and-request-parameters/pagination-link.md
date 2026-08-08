# `PaginationLink`

**Namespace:** `drogular`  
**Header:** `<drogular/pagination_model.hpp>`  
**Kind:** Struct

## Purpose

`PaginationLink` describes one numbered link in a pagination control.

---

## Synopsis

```cpp
struct PaginationLink {
    int number{1};
    std::string url;
    bool current{false};
};
```

---

## Members

### `number`

1-based page number represented by the link.

### `url`

Application-generated URL for the page.

### `current`

True when the link represents the current page.

---

## JSON Conversion

```cpp
Json::Value toJson(
    const PaginationLink& link
);
```

Produces an object with:

```text
number
url
current
```

This conversion participates in Drogular's JSON conversion helpers used by `RenderContext::setJson()`.

---

## Related Types

- [`PaginationModel`](pagination-model.md)
- [`makePaginationModel()`](pagination-model.md#makepaginationmodel)
