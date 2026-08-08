# Static Files

Drogular provides a static-file pipeline for mapping URL prefixes to filesystem directories, protecting mapped roots from path traversal, selecting MIME types, and applying browser cache validators.

The high-level flow is:

```text
App::staticFiles()
        │
        ▼
ApplicationOptions
        │
        ▼
StaticFileMapping
        │
        ▼
Router::staticFiles()
        │
        ▼
StaticFileResolver
        │
        ├── ETag
        ├── Last-Modified
        └── Cache-Control
        │
        ▼
StaticFileResponse
```

Applications normally configure this subsystem through [`App`](../application/app.md). The lower-level types are public for cases where application code needs the same resolution or response behavior outside a registered static-file route.

---

## Types

- [`StaticFileMapping`](static-file-mapping.md) — maps a URL route prefix to a filesystem directory.
- [`StaticFileResolver`](static-file-resolver.md) — resolves a relative file path while enforcing the configured root directory.
- [`StaticFileResponseOptions`](static-file-response-options.md) — controls Cache-Control, ETag, and Last-Modified response behavior.
- [`StaticFileResponse`](static-file-response.md) — creates file and 304 responses.
- [`StaticFileCacheProfile`](static-file-cache-profile.md) — applies predefined static-file cache strategies.
- [`StaticFileContentType`](static-file-content-type.md) — maps supported file extensions to MIME types.
- [`StaticFileEtag`](static-file-etag.md) — creates weak ETag values from file metadata.
- [`StaticFileLastModified`](static-file-last-modified.md) — creates and compares Last-Modified values.

---

## Typical Configuration

TodoPWA registers its public directory and selects the development cache profile:

```cpp
app.staticFiles(
    "/assets",
    "examples/todo_pwa/public"
)
.staticFileCacheProfile(
    drogular::StaticFileCacheProfile::Development
);
```

A request such as:

```text
/assets/logo.svg
```

is resolved relative to:

```text
examples/todo_pwa/public
```

The router rejects paths that escape this directory and returns `404` for missing or non-regular files.

---

## Conditional Requests

When enabled, the router checks validators before creating the normal file response:

```text
If-None-Match
      │
      ├── matches ETag ─────► 304
      │
      ▼
If-Modified-Since
      │
      ├── matches value ────► 304
      │
      ▼
StaticFileResponse::create()
      │
      ▼
200 file response
```

The current implementation uses exact string comparisons for both validators. It does not parse ETag lists or HTTP-date values.

---

## Cache Profiles

| Profile | Cache-Control | ETag | Last-Modified |
|---|---|---|---|
| `Disabled` | `no-store` | off | off |
| `Development` | `no-store` | on | on |
| `Production` | `public, max-age=86400` | on | on |

The production profile also resets max-age to 24 hours.

---

## See Also

### API Reference

- [`App`](../application/app.md)
- [`ApplicationOptions`](../application/application-options.md)
- [`Router`](../routing/router.md)
- [`PWA`](../pwa/README.md)
