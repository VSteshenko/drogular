# Localization

The localization API resolves the active request locale and translates application keys through a registered `TranslationProvider`.

`LocaleSupport` reads the current locale, `TranslationSupport` coordinates translation lookup, and `RenderContext` exposes convenience methods for pages and components.

---

## Types

- [`TranslationProvider`](translation-provider.md) — application-defined interface for resolving localized text.
- [`TranslationSupport`](translation-support.md) — translates keys using the current or an explicit locale.
- [`LocaleSupport`](locale-support.md) — resolves the active locale from the request.

---

## Typical Flow

```text
Request cookie: lang
        │
        ▼
LocaleSupport
        │
        ▼
TranslationSupport
        │
        ▼
TranslationProvider
        │
        ▼
Localized text
```

`RenderContext::translate()` uses this flow automatically. `RenderContext::setTranslated()` and `RenderContext::setTranslations()` store translated values directly in the rendering context.

---

## Cookbook

- [Localization](../../cookbook/localization.md)

## Related API

- `RenderContext` *(Rendering reference coming soon)*
