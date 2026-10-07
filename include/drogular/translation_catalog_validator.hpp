#pragma once

#include <drogular/file_translation_provider.hpp>

#include <map>
#include <string>
#include <vector>

namespace drogular {

struct TranslationLocaleValidation {
    std::vector<std::string> missingKeys;
    std::vector<std::string> extraKeys;

    bool valid() const noexcept {
        return missingKeys.empty() && extraKeys.empty();
    }
};

struct TranslationValidationResult {
    std::string referenceLocale;
    std::map<std::string, TranslationLocaleValidation> locales;

    bool valid() const noexcept;
};

class TranslationCatalogValidator final {
public:
    using Catalog = FileTranslationProvider::Catalog;

    static TranslationValidationResult validate(
        const Catalog& catalog,
        const std::string& referenceLocale
    );

    static TranslationValidationResult validate(
        const FileTranslationProvider& provider
    );
};

} // namespace drogular