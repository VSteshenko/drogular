# FileTranslationProvider

`drogular::FileTranslationProvider` is a ready-to-use `TranslationProvider` that loads translations from JSON files on disk.

Include it with:

```cpp
#include <drogular/file_translation_provider.hpp>
```

## Resource layout

Pass a directory containing one `.json` file per locale. The file name without the extension becomes the locale name.

```text
resources/localization/
├── de.json
└── en.json
```

Each file must contain a flat JSON object whose keys and values are strings:

```json
{
  "common.save": "Save",
  "projects.title": "Projects"
}
```

Nested objects and non-string translation values are not supported.

## Construction

```cpp
auto translations = std::make_shared<drogular::FileTranslationProvider>(
    "resources/localization",
    "en"
);
```

The second argument is the default locale and defaults to `"en"`.

Construction loads the catalog immediately. It throws when the translation directory does not exist, the default locale 
file is missing, a JSON file cannot be parsed, the JSON root is not an object, or a translation value is not a string. 
An empty default locale is rejected with `std::invalid_argument`.

This fail-fast behavior keeps file and catalog errors out of request-time translation.

## Translation fallback

`translate(locale, key)` resolves a value in this order:

1. the requested locale;
2. the default locale;
3. the key itself.

For example, if `de.json` does not define `projects.search` but `en.json` does, translating that key for `de` returns 
the English value. If neither locale defines it, the result is `"projects.search"`.

## Catalog inspection

The provider exposes the loaded catalog for diagnostics and tooling:

```cpp
translations->directory();
translations->defaultLocale();
translations->catalog();
translations->locales();
translations->keys("de");
```

`catalog()` returns the complete locale-to-translation map. `locales()` returns the loaded locale names, and `keys(locale)` 
returns the keys present in that locale; an unknown locale produces an empty key list.

These inspection APIs allow validation tools to compare locale catalogs without reparsing the source files.

## Runtime behavior

Translation files are read during construction and retained in memory. `translate()` does not access the filesystem. 
Changes to translation files therefore require creating a new `FileTranslationProvider` instance.

The provider does not choose the current request locale. Locale resolution remains the responsibility of 
`LocaleSupport` / `TranslationSupport` and the application.

## See also

- [`TranslationProvider`](translation-provider.md)
- [`TranslationCatalogValidator`](translation-catalog-validator.md)
- [`drogular-l10n-check`](localization-check.md)
- [`TranslationSupport`](translation-support.md)
- [`LocaleSupport`](locale-support.md)