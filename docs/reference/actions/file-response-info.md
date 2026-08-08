# `FileResponseInfo`

**Namespace:** `drogular`  
**Header:** `<drogular/action_result.hpp>`  
**Kind:** Struct

## Purpose

`FileResponseInfo` stores metadata for file responses returned from an action.

---

## Synopsis

```cpp
struct FileResponseInfo {
    std::filesystem::path path;
    std::string downloadName;
    bool forceDownload = false;
};
```

---

## Members

### `path`

Filesystem path passed to `StaticFileResponse::create()` during HTTP conversion.

### `downloadName`

Filename placed in the `Content-Disposition` header for forced downloads.

For ordinary file responses this value is empty.

### `forceDownload`

When `true`, `toHttpResponse()` adds an attachment `Content-Disposition` header.

---

## Factory Mapping

`ActionResult::file(path)` produces:

```text
path          = path
downloadName  = ""
forceDownload = false
```

`ActionResult::download(path, name)` produces:

```text
path          = path
downloadName  = name
forceDownload = true
```

---

## Related Types

- [`ActionResult`](action-result.md)
- [`ActionResultType`](action-result-type.md)
- [`toHttpResponse()`](action-response.md)
