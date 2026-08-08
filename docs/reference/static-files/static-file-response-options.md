# `StaticFileResponseOptions`

**Namespace:** `drogular`  
**Header:** `<drogular/static_file_response.hpp>`  
**Kind:** Struct

## Purpose

`StaticFileResponseOptions` controls the cache-related headers produced by [`StaticFileResponse`](static-file-response.md).

---

## Synopsis

```cpp
struct StaticFileResponseOptions {
    bool cacheEnabled = true;
    std::chrono::seconds maxAge = std::chrono::hours(24);
    bool etagEnabled = true;
    bool lastModifiedEnabled = true;
};
```

---

## Members

### `cacheEnabled`

When `true`, `StaticFileResponse::create()` adds:

```text
Cache-Control: public, max-age=<seconds>
```

When `false`, it adds:

```text
Cache-Control: no-store
```

The header is always emitted; disabling cache does not omit it.

### `maxAge`

Used as the numeric `max-age` value when `cacheEnabled` is `true`.

The default is 24 hours.

### `etagEnabled`

Controls whether an `ETag` header is generated through [`StaticFileEtag`](static-file-etag.md).

### `lastModifiedEnabled`

Controls whether a `Last-Modified` header is generated through [`StaticFileLastModified`](static-file-last-modified.md).

---

## Example

```cpp
drogular::StaticFileResponseOptions options;
options.cacheEnabled = true;
options.maxAge = std::chrono::seconds(60);
options.etagEnabled = true;
options.lastModifiedEnabled = false;

const auto response =
    drogular::StaticFileResponse::create(
        path,
        options
    );
```

---

## Related Types

- [`StaticFileResponse`](static-file-response.md)
- [`StaticFileCacheProfile`](static-file-cache-profile.md)
- [`ApplicationOptions`](../application/application-options.md)
