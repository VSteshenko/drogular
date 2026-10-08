# Localization

The localization API resolves the active request locale and translates application keys through a registered `TranslationProvider`.

`LocaleSupport` reads the current locale, `TranslationSupport` coordinates translation lookup, and `RenderContext` exposes convenience methods for pages and components.

---

## Types

- [`TranslationProvider`](translation-provider.md) — application-defined interface for resolving localized text.
- [`FileTranslationProvider`](file-translation-provider.md) — loads a flat JSON translation catalog from one file per locale.
- [`TranslationCatalogValidator`](translation-catalog-validator.md) — compares locale key sets against a reference locale.
- [`drogular-l10n-check`](localization-check.md) — validates file-based catalogs from the command line and CI.
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

- [`RenderContext`](../rendering/render-context.md)
