#include <drogular/translation_catalog_validator.hpp>

#include <algorithm>
#include <stdexcept>

namespace drogular {

bool TranslationValidationResult::valid() const noexcept {
    return std::all_of(
        locales.begin(),
        locales.end(),
        [](const auto& entry) { return entry.second.valid(); }
    );
}

TranslationValidationResult TranslationCatalogValidator::validate(
    const Catalog& catalog,
    const std::string& referenceLocale
) {
    const auto reference = catalog.find(referenceLocale);
    if (reference == catalog.end()) {
        throw std::invalid_argument(
            "Translation reference locale is not present in catalog: " + referenceLocale
        );
    }

    TranslationValidationResult result;
    result.referenceLocale = referenceLocale;

    for (const auto& [locale, translations] : catalog) {
        if (locale == referenceLocale) {
            continue;
        }

        TranslationLocaleValidation validation;

        for (const auto& [key, value] : reference->second) {
            (void)value;
            if (!translations.contains(key)) {
                validation.missingKeys.push_back(key);
            }
        }

        for (const auto& [key, value] : translations) {
            (void)value;
            if (!reference->second.contains(key)) {
                validation.extraKeys.push_back(key);
            }
        }

        result.locales.emplace(locale, std::move(validation));
    }

    return result;
}

TranslationValidationResult TranslationCatalogValidator::validate(
    const FileTranslationProvider& provider
) {
    return validate(provider.catalog(), provider.defaultLocale());
}

} // namespace drogular