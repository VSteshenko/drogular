# `StaticFileLastModified`

**Namespace:** `drogular`  
**Header:** `<drogular/static_file_last_modified.hpp>`  
**Kind:** Utility class

## Purpose

`StaticFileLastModified` converts file modification time into an HTTP-date string and provides the comparison used for `If-Modified-Since` handling.

---

## Public API

### `create()`

```cpp
static std::string create(
    const std::filesystem::path& path
);
```

Reads `std::filesystem::last_write_time()` and formats the value as GMT using:

```text
%a, %d %b %Y %H:%M:%S GMT
```

Example shape:

```text
Sat, 08 Aug 2026 18:30:00 GMT
```

Filesystem errors are not caught and may propagate as `std::filesystem` exceptions.

### `matches()`

```cpp
static bool matches(
    const std::filesystem::path& path,
    const std::string& requestValue
);
```

Returns `true` only when `requestValue` is non-empty and is exactly equal to the value returned by `create(path)`.

The helper does not parse HTTP dates or perform tolerance-based timestamp comparison.

---

## Router Behavior

`Router::staticFiles()` currently performs the equivalent exact string comparison directly when processing `If-Modified-Since`.

A match produces a `304 Not Modified` response.

---

## Related Types

- [`StaticFileResponse`](static-file-response.md)
- [`StaticFileEtag`](static-file-etag.md)
- [`Router`](../routing/router.md)
