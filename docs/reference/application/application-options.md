# `ApplicationOptions`

**Namespace:** `drogular`  
**Header:** `<drogular/application_options.hpp>`  
**Kind:** Class

## Purpose

`ApplicationOptions` stores application-wide configuration used by `App`, routing, template loading, static file responses, and PWA setup.

A mutable instance is available through `App::options()`. Most common settings also have fluent convenience methods on `App`.

---

## Synopsis

```cpp
class ApplicationOptions {
public:
    void setTemplateRoot(std::filesystem::path root);
    const std::filesystem::path& templateRoot() const;

    void setTemplateCacheEnabled(bool enabled);
    bool templateCacheEnabled() const;

    void addStaticFiles(
        std::string routePrefix,
        std::filesystem::path directory
    );

    const std::vector<StaticFileMapping>& staticFiles() const;

    void setStaticFileCacheEnabled(bool enabled);
    bool staticFileCacheEnabled() const;

    void setStaticFileCacheMaxAge(
        std::chrono::seconds maxAge
    );

    std::chrono::seconds staticFileCacheMaxAge() const;

    void setStaticFileEtagEnabled(bool enabled);
    bool staticFileEtagEnabled() const;

    void setStaticFileLastModifiedEnabled(bool enabled);
    bool staticFileLastModifiedEnabled() const;

    void setStaticFileCacheProfile(
        StaticFileCacheProfile profile
    );

    void setServiceWorker(std::filesystem::path path);

    const std::optional<std::filesystem::path>&
    serviceWorkerPath() const;
};
```

---

## Default Values

| Option | Default |
|---|---|
| Template root | empty path |
| Template cache | enabled |
| Static file mappings | none |
| `Cache-Control` | enabled |
| Static file max-age | 24 hours |
| ETag | enabled |
| Last-Modified | enabled |
| Service worker path | unset |

---

## Public API

### Template configuration

#### `setTemplateRoot()`

```cpp
void setTemplateRoot(
    std::filesystem::path root
);
```

Sets the directory used as the base for template and layout paths.

#### `templateRoot()`

```cpp
const std::filesystem::path& templateRoot() const;
```

Returns the configured template root.

#### `setTemplateCacheEnabled()`

```cpp
void setTemplateCacheEnabled(bool enabled);
```

Enables or disables template source caching.

Disabling the cache causes template sources to be read from disk again during later rendering operations.

#### `templateCacheEnabled()`

```cpp
bool templateCacheEnabled() const;
```

Returns whether template source caching is enabled.

### Static file mappings

#### `addStaticFiles()`

```cpp
void addStaticFiles(
    std::string routePrefix,
    std::filesystem::path directory
);
```

Appends a route-prefix to directory mapping.

Mappings are registered with the router when `App::run()` starts the application.

#### `staticFiles()`

```cpp
const std::vector<StaticFileMapping>& staticFiles() const;
```

Returns all configured [`StaticFileMapping`](../static-files/static-file-mapping.md) values in registration order.

### Static file cache behavior

#### `setStaticFileCacheEnabled()` / `staticFileCacheEnabled()`

Controls whether `Cache-Control` headers are added to static file responses.

#### `setStaticFileCacheMaxAge()` / `staticFileCacheMaxAge()`

Sets and returns the `Cache-Control` max-age value.

#### `setStaticFileEtagEnabled()` / `staticFileEtagEnabled()`

Controls ETag generation for static file responses.

#### `setStaticFileLastModifiedEnabled()` / `staticFileLastModifiedEnabled()`

Controls Last-Modified headers for static file responses.

#### `setStaticFileCacheProfile()`

```cpp
void setStaticFileCacheProfile(
    StaticFileCacheProfile profile
);
```

Applies a predefined [`StaticFileCacheProfile`](../static-files/static-file-cache-profile.md).

| Profile | Cache-Control | Max-age | ETag | Last-Modified |
|---|---:|---:|---:|---:|
| `Disabled` | off | unchanged | off | off |
| `Development` | off | unchanged | on | on |
| `Production` | on | 24 hours | on | on |

### Service worker

#### `setServiceWorker()`

```cpp
void setServiceWorker(
    std::filesystem::path path
);
```

Stores the file path that `App` later registers at `/service-worker.js`.

#### `serviceWorkerPath()`

```cpp
const std::optional<std::filesystem::path>&
serviceWorkerPath() const;
```

Returns the configured service worker path, or an empty optional when none is configured.

---

## Example

```cpp
drogular::App app;

auto& options = app.options();

options.setTemplateRoot("templates");
options.setTemplateCacheEnabled(false);
options.addStaticFiles("/assets", "public");
options.setStaticFileCacheProfile(
    drogular::StaticFileCacheProfile::Development
);
options.setServiceWorker("public/service-worker.js");
```

Equivalent convenience methods are available for most settings directly on `App`.

---

## Related Types

- [`App`](app.md)
- `StaticFileMapping`
- `StaticFileCacheProfile`

## See Also

- [Static Files](../static-files/README.md)
- [Static Files API Reference](../static-files/README.md)
- [PWA API Reference](../pwa/README.md)
