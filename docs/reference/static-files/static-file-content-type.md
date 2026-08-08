# `StaticFileContentType`

**Namespace:** `drogular`  
**Header:** `<drogular/static_file_content_type.hpp>`  
**Kind:** Utility class

## Purpose

`StaticFileContentType` maps known file extensions to MIME content types used by [`StaticFileResponse`](static-file-response.md).

Extension matching is case-insensitive.

---

## Public API

### `fromPath()`

```cpp
static std::string fromPath(
    const std::filesystem::path& path
);
```

Returns the MIME type associated with the path extension.

The current mapping is:

| Extension | MIME type |
|---|---|
| `.html` | `text/html` |
| `.css` | `text/css` |
| `.js` | `application/javascript` |
| `.json` | `application/json` |
| `.webmanifest` | `application/manifest+json` |
| `.svg` | `image/svg+xml` |
| `.png` | `image/png` |
| `.jpg`, `.jpeg` | `image/jpeg` |
| `.ico` | `image/x-icon` |
| `.webp` | `image/webp` |
| `.txt` | `text/plain` |
| `.pdf` | `application/pdf` |

Unknown extensions return:

```text
application/octet-stream
```

---

## Example

```cpp
const auto type =
    drogular::StaticFileContentType::fromPath(
        "assets/LOGO.SVG"
    );

// image/svg+xml
```

---

## Related Types

- [`StaticFileResponse`](static-file-response.md)
