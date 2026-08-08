# `PwaOptions`

**Namespace:** `drogular`  
**Header:** `<drogular/pwa_options.hpp>`  
**Kind:** Configuration struct

## Purpose

`PwaOptions` stores the paths and metadata used by `PwaPageSupport` when it generates PWA-related template values.

---

## Synopsis

```cpp
struct PwaOptions {
    std::string manifestPath =
        "/assets/manifest.webmanifest";

    std::string faviconPath =
        "/assets/favicon.ico";

    std::string faviconType =
        "image/x-icon";

    std::string themeColor =
        "#4f46e5";

    std::string serviceWorkerPath =
        "/service-worker.js";
};
```

---

## Members

### `manifestPath`

```cpp
std::string manifestPath =
    "/assets/manifest.webmanifest";
```

URL embedded in the generated manifest `<link>` tag.

Drogular does not create or validate a manifest at this path. Expose the manifest separately, normally with `App::staticFiles()`.

### `faviconPath`

```cpp
std::string faviconPath =
    "/assets/favicon.ico";
```

URL embedded in the generated favicon `<link>` tag.

### `faviconType`

```cpp
std::string faviconType =
    "image/x-icon";
```

MIME type embedded in the favicon `<link>` tag.

### `themeColor`

```cpp
std::string themeColor =
    "#4f46e5";
```

Value embedded in the generated `theme-color` meta tag.

### `serviceWorkerPath`

```cpp
std::string serviceWorkerPath =
    "/service-worker.js";
```

URL embedded in the generated service-worker registration script.

This value does **not** register a server route. `App::serviceWorker()` currently publishes its configured file at the fixed route `/service-worker.js`. If you set a custom `serviceWorkerPath`, the application must expose a matching route by some other mechanism.

---

## Example

```cpp
drogular::PwaOptions options;
options.manifestPath = "/assets/app.webmanifest";
options.faviconPath = "/assets/app-icon.png";
options.faviconType = "image/png";
options.themeColor = "#111827";

// Keep this aligned with App::serviceWorker().
options.serviceWorkerPath = "/service-worker.js";

drogular::PwaPageSupport::apply(
    context,
    options
);
```

---

## Behavior

`PwaOptions` contains configuration data only. It performs no validation and registers no routes or files.

All strings are passed to `PwaHtml` or `PwaScripts` as provided. They should therefore be trusted application-controlled values rather than untrusted request input.

---

## Related Types

- [`PwaPageSupport`](../pages/pwa-page-support.md)
- [`PwaHtml`](pwa-html.md)
- [`PwaScripts`](pwa-scripts.md)
- [`App`](../application/app.md)
