#pragma once

#include <drogular/translation_provider.hpp>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace drogular {

/**
 * Loads translations from one JSON file per locale.
 *
 * Each file must contain a flat JSON object whose values are strings.
 * The file name without the .json extension is used as the locale name.
 */
class FileTranslationProvider final : public TranslationProvider {
public:
    using TranslationMap = std::map<std::string, std::string>;
    using Catalog = std::map<std::string, TranslationMap>;

    explicit FileTranslationProvider(
        std::filesystem::path directory,
        std::string defaultLocale = "en"
    );

    std::string translate(
        const std::string& locale,
        const std::string& key
    ) const override;

    const std::filesystem::path& directory() const noexcept;
    const std::string& defaultLocale() const noexcept;
    const Catalog& catalog() const noexcept;
    std::vector<std::string> locales() const;
    std::vector<std::string> keys(const std::string& locale) const;

private:
    std::filesystem::path directory_;
    std::string defaultLocale_;
    Catalog catalog_;
};

} // namespace drogular