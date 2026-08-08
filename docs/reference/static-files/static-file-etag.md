# `StaticFileEtag`

**Namespace:** `drogular`  
**Header:** `<drogular/static_file_etag.hpp>`  
**Kind:** Utility class

## Purpose

`StaticFileEtag` creates weak ETag values for files using filesystem metadata.

It is used by static-file routes for conditional `If-None-Match` requests.

---

## Public API

### `create()`

```cpp
static std::string create(
    const std::filesystem::path& path
);
```

Builds an ETag from:

- file size;
- the raw `last_write_time()` clock count.

The generated shape is:

```text
W/"<size>-<last-write-time>"
```

For example:

```text
W/"512-1736284190523456000"
```

The ETag is metadata-based. It is not a hash of the file contents.

A metadata change can therefore change the ETag, while identical metadata can produce the same ETag without comparing file bytes.

Filesystem metadata errors are not caught by this helper and may propagate as `std::filesystem` exceptions.

---

## Conditional Request Behavior

`Router::staticFiles()` compares the request `If-None-Match` value to the generated ETag using exact string equality.

The current implementation does not parse:

- comma-separated ETag lists;
- wildcard `*` semantics;
- alternate weak/strong representations.

---

## Related Types

- [`StaticFileResponse`](static-file-response.md)
- [`StaticFileLastModified`](static-file-last-modified.md)
- [`Router`](../routing/router.md)
