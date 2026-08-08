# `StaticFileCacheProfile`

**Namespace:** `drogular`  
**Header:** `<drogular/static_file_cache_profile.hpp>`  
**Kind:** Enum class

## Purpose

`StaticFileCacheProfile` groups common static-file caching settings into predefined application profiles.

Profiles are applied through `App::staticFileCacheProfile()` or `ApplicationOptions::setStaticFileCacheProfile()`.

---

## Values

### `Disabled`

Applies:

```text
Cache-Control: no-store
ETag: disabled
Last-Modified: disabled
```

Internally this sets:

```cpp
cacheEnabled = false;
etagEnabled = false;
lastModifiedEnabled = false;
```

It does not reset the stored max-age value.

### `Development`

Applies:

```text
Cache-Control: no-store
ETag: enabled
Last-Modified: enabled
```

This keeps validation metadata available while disabling browser cache storage.

It does not reset the stored max-age value.

### `Production`

Applies:

```text
Cache-Control: public, max-age=86400
ETag: enabled
Last-Modified: enabled
```

The profile explicitly resets max-age to 24 hours.

---

## Example

```cpp
app.staticFileCacheProfile(
    drogular::StaticFileCacheProfile::Development
);
```

TodoPWA and PortalDemo use the development profile in their current application setup.

---

## Related Types

- [`ApplicationOptions`](../application/application-options.md)
- [`StaticFileResponseOptions`](static-file-response-options.md)
- [`StaticFileResponse`](static-file-response.md)
