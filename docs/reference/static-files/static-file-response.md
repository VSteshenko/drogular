# `StaticFileResponse`

**Namespace:** `drogular`  
**Header:** `<drogular/static_file_response.hpp>`  
**Kind:** Utility class

## Purpose

`StaticFileResponse` creates Drogon file responses with Drogular's MIME-type and cache-header behavior, and can create an explicit `304 Not Modified` response.

Registered static-file routes use this helper after path resolution and conditional-request checks.

---

## Public API

### `create()`

```cpp
static drogon::HttpResponsePtr create(
    const std::filesystem::path& path,
    StaticFileResponseOptions options = {}
);
```

Creates a Drogon file response for `path`.

The method:

1. creates a file response through `drogon::HttpResponse::newFileResponse()`;
2. determines `Content-Type` through [`StaticFileContentType`](static-file-content-type.md);
3. adds `Cache-Control`;
4. optionally adds `ETag`;
5. optionally adds `Last-Modified`.

Default headers include:

```text
Cache-Control: public, max-age=86400
ETag: ...
Last-Modified: ... GMT
```

`create()` assumes the supplied path is a valid file path. The normal router pipeline validates it first with [`StaticFileResolver`](static-file-resolver.md).

Filesystem metadata operations used by ETag and Last-Modified generation may propagate `std::filesystem` exceptions.

### `notModified()`

```cpp
static drogon::HttpResponsePtr notModified(
    std::string etag = "",
    std::string lastModified = ""
);
```

Creates an empty response with status:

```text
304 Not Modified
```

`ETag` and `Last-Modified` are added only when the corresponding argument is non-empty.

```cpp
return drogular::StaticFileResponse::notModified(
    etag,
    lastModified
);
```

---

## Router Behavior

`Router::staticFiles()` performs conditional checks before calling `create()`.

ETag is checked first. A matching `If-None-Match` produces `notModified(etag)`.

If ETag did not match, Last-Modified is checked next. The current router implementation calls `notModified()` with the ETag value only in that branch; it does not pass the computed Last-Modified value into the 304 response.

---

## Example

```cpp
drogular::StaticFileResponseOptions options;
options.cacheEnabled = false;
options.etagEnabled = true;

const auto response =
    drogular::StaticFileResponse::create(
        "public/logo.svg",
        options
    );
```

---

## Related Types

- [`StaticFileResponseOptions`](static-file-response-options.md)
- [`StaticFileResolver`](static-file-resolver.md)
- [`StaticFileContentType`](static-file-content-type.md)
- [`StaticFileEtag`](static-file-etag.md)
- [`StaticFileLastModified`](static-file-last-modified.md)
- [`Router`](../routing/router.md)
