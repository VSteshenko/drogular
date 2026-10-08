# TranslationCatalogValidator

`drogular::TranslationCatalogValidator` compares the key sets in a loaded translation catalog against a reference locale.

Include it with:

```cpp
#include <drogular/translation_catalog_validator.hpp>
```

## Validation model

The reference locale defines the expected key set. Every other locale is checked independently for:

- **missing keys** — present in the reference locale but absent from the checked locale;
- **extra keys** — present in the checked locale but absent from the reference locale.

Validation compares catalog structure only. It does not scan C++ sources or templates for translation-key usage, so 
an `extra` key is not necessarily an unused key.

## Validate a catalog

```cpp
const auto result = drogular::TranslationCatalogValidator::validate(
    catalog,
    "en"
);

if (!result.valid()) {
    // Inspect result.locales.
}
```

The reference locale must exist in the catalog. Otherwise validation throws `std::invalid_argument`.

`TranslationValidationResult::referenceLocale` records the selected reference locale. `locales` contains one 
`TranslationLocaleValidation` entry for every non-reference locale. Each entry exposes `missingKeys`, `extraKeys`, and `valid()`.

## Validate a FileTranslationProvider

A convenience overload uses the provider's default locale as the reference locale:

```cpp
drogular::FileTranslationProvider translations(
    "resources/localization",
    "en"
);

const auto result =
    drogular::TranslationCatalogValidator::validate(translations);
```

This reuses the catalog already loaded by `FileTranslationProvider`; validation does not read or parse the translation files again.

## Deterministic results

The catalog and locale result maps use ordered containers, so locale and key reporting is deterministic. This makes 
the validator suitable for tests, command-line output, and CI gates.

## See also

- [`FileTranslationProvider`](file-translation-provider.md)
- [`drogular-l10n-check`](localization-check.md)
- [`TranslationProvider`](translation-provider.md)