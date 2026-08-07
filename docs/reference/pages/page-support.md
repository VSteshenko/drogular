# `PageSupport`

**Namespace:** `drogular`  
**Header:** `<drogular/page_support.hpp>`  
**Kind:** Utility class

## Purpose

`PageSupport` adds common page values to a `RenderContext`.

It also applies the default PWA values provided by `PwaPageSupport`.

---

## Public API

### `apply()`

```cpp
static void apply(
    RenderContext& context,
    const std::string& pageTitle = ""
);
```

Adds these context values:

| Key | Value |
|---|---|
| `pageTitle` | The supplied page title. |
| `currentPath` | `request->path()` when a request is attached; otherwise `/`. |
| `manifestLink` | Default manifest `<link>` markup. |
| `faviconLink` | Default favicon `<link>` markup. |
| `themeColorMeta` | Default theme-color `<meta>` markup. |
| `serviceWorkerRegistration` | Default service-worker registration script. |

The PWA values are added by calling `PwaPageSupport::apply(context)` with default `PwaOptions`.

---

## Example

```cpp
class DashboardPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        drogular::PageSupport::apply(
            context,
            "Dashboard"
        );
    }

    std::string templateHtml() const override {
        return R"(
<h1>{{ pageTitle }}</h1>
<p>{{ currentPath }}</p>
{{{ manifestLink }}}
{{{ faviconLink }}}
{{{ themeColorMeta }}}
{{{ serviceWorkerRegistration }}}
)";
    }
};
```

Use triple-brace template output for generated HTML snippets when they must not be escaped.

---

## Behavior

`PageSupport` overwrites existing values with the same keys.

Calling it without an attached request stores `/` as `currentPath`. This is the behavior used by page testing helpers that render without a live HTTP request.

The helper does not localize `pageTitle`. Applications that use translation keys should translate the title before passing it or provide an application-specific page support wrapper.

---

## Related Types

- [`PwaPageSupport`](pwa-page-support.md)
- [`Page`](page.md)
- [`TemplatePage`](template-page.md)
- [`RenderContext`](../rendering/render-context.md)
- `PwaOptions` *(coming soon)*
