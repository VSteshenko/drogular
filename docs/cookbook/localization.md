# Localization

## Problem

**Need to support multiple languages?**

This guide shows how to separate user-visible text from application logic using Drogular's localization infrastructure.

---

## Recommended Solution

Register a `TranslationProvider` and resolve translated values through `RenderContext`.

For file-based application resources, use the built-in `FileTranslationProvider`. Custom providers remain available when 
translations come from another storage system or require a different lookup policy.

---

## How It Works

### Translation Provider

`TranslationProvider` defines a simple interface for resolving localized text.

Applications are free to implement the interface using any storage mechanism. Drogular also provides `FileTranslationProvider`, 
which loads one flat JSON file per locale and keeps the catalog in memory.

```cpp
class TranslationProvider
{
public:
    virtual ~TranslationProvider() = default;

    virtual std::string translate(
        const std::string& locale,
        const std::string& key
    ) const = 0;
};
```

---

### Registering the Provider

Create one file per locale:

```text
resources/localization/
├── de.json
└── en.json
```

For example, `en.json` can contain:

```json
{
  "nav.dashboard": "Dashboard",
  "users.title": "Users",
  "common.previous": "Previous"
}
```

Translation files are flat JSON objects with string values. Register the provider through dependency injection:

```cpp
app.services().addFactory<drogular::TranslationProvider>(
    drogular::ServiceLifetime::Singleton,
    [] {
        return std::make_shared<drogular::FileTranslationProvider>(
            "resources/localization",
            "en"
        );
    }
);
```

Using a singleton ensures that the application shares a single translation provider.

---

### Translation Keys

Translation keys should reflect the application domain rather than the organization of translation files.

For example:

```text
nav.dashboard
users.title
users.search.label
departments.edit.title
common.previous
```

A consistent naming scheme keeps translations stable as the application evolves.

---

## Example

Translate user-visible text through `RenderContext`.

```cpp
const auto title =
    context.translate("users.title");
```

`RenderContext` resolves the current locale and delegates the lookup to the registered translation provider.

```text
RenderContext
      │
      ▼
Current Locale
      │
      ▼
TranslationProvider
      │
      ▼
Localized Text
```

Translated values are then passed to the template.

```cpp
context.set(
    "pageTitle",
    context.translate("users.title")
);
```

When several values must be translated, register them together.

```cpp
context.setTranslations({
    {"pageTitle", "users.title"},
    {"navUsers", "nav.users"},
    {"navDepartments", "nav.departments"},
    {"commonPrevious", "common.previous"},
    {"commonNext", "common.next"}
});
```

This keeps page initialization concise while making every translated value explicit.

---

### Fallback Behavior

Fallback behavior is defined by the translation provider implementation.

`FileTranslationProvider` first searches the requested locale. If the locale or translation key cannot be found, it 
searches the configured default locale. If the key is also missing there, it returns the key itself.

Applications may implement a different fallback strategy while using the same `TranslationProvider` interface.

---

## Validate Translation Catalogs

For file-based catalogs, use `drogular-l10n-check` during development or CI:

```bash
drogular-l10n-check resources/localization --reference en
```

The reference locale defines the expected key set. The command reports keys missing from another locale and keys that 
exist only in another locale. It returns a non-zero exit code when the catalog is incomplete or structurally inconsistent.

For validation inside C++ tooling or tests, use `TranslationCatalogValidator` directly with a loaded catalog or 
`FileTranslationProvider`. Validation compares locale key sets; it does not scan application source files for unused translation keys.

## Best Practices

- Keep translation keys stable.
- Organize translation keys by application domain.
- Keep user-visible text outside pages and components.
- Register translations through `TranslationProvider`.
- Keep locale selection separate from translation lookup.
- Let the translation provider define storage and fallback behavior.

---

## See Also

### API Reference

- [`TranslationProvider`](../reference/localization/translation-provider.md)
- [`FileTranslationProvider`](../reference/localization/file-translation-provider.md)
- [`TranslationCatalogValidator`](../reference/localization/translation-catalog-validator.md)
- [`drogular-l10n-check`](../reference/localization/localization-check.md)
- [`TranslationSupport`](../reference/localization/translation-support.md)
- [`LocaleSupport`](../reference/localization/locale-support.md)
- [`RenderContext`](../reference/rendering/render-context.md)
