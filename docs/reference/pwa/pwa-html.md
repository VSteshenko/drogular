# `PwaHtml`

**Namespace:** `drogular`  
**Header:** `<drogular/pwa_html.hpp>`  
**Kind:** Utility class

## Purpose

`PwaHtml` generates small HTML snippets used by PWA-enabled layouts.

---

## Public API

### `manifestLink()`

```cpp
static std::string manifestLink(
    const std::string& path =
        "/assets/manifest.webmanifest"
);
```

Returns:

```html
<link rel="manifest" href="/assets/manifest.webmanifest">
```

with the supplied `path` inserted into `href`.

### `favicon()`

```cpp
static std::string favicon(
    const std::string& path =
        "/assets/favicon.ico",
    const std::string& type =
        "image/x-icon"
);
```

Returns a favicon `<link>` tag containing the supplied MIME type and path.

Default output:

```html
<link rel="icon" type="image/x-icon" href="/assets/favicon.ico">
```

### `themeColor()`

```cpp
static std::string themeColor(
    const std::string& color
);
```

Returns a `theme-color` meta tag:

```html
<meta name="theme-color" content="#4f46e5">
```

---

## Behavior

The helpers perform direct string concatenation. They do not HTML-escape paths, MIME types, or color values.

Use trusted application-controlled strings. Passing untrusted text can create malformed markup or injection vulnerabilities when the returned snippet is emitted unescaped.

The returned strings are markup fragments only. The helpers do not verify that the referenced manifest or icon exists.

---

## Example

```cpp
const auto manifest =
    drogular::PwaHtml::manifestLink(
        "/assets/manifest.webmanifest"
    );

const auto favicon =
    drogular::PwaHtml::favicon(
        "/assets/favicon.ico",
        "image/x-icon"
    );

const auto theme =
    drogular::PwaHtml::themeColor(
        "#4f46e5"
    );
```

`PwaPageSupport::apply()` normally calls these helpers and stores their results in the render context automatically.

---

## Related Types

- [`PwaOptions`](pwa-options.md)
- [`PwaPageSupport`](../pages/pwa-page-support.md)
- [`PwaScripts`](pwa-scripts.md)
