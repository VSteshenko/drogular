#include <drogular/file_translation_provider.hpp>

#include <json/json.h>

#include <fstream>
#include <stdexcept>
#include <utility>

namespace drogular {
namespace {

FileTranslationProvider::TranslationMap loadTranslations(
    const std::filesystem::path& path
) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error(
            "Unable to open translation file: " + path.string()
        );
    }

    Json::CharReaderBuilder builder;
    Json::Value root;
    std::string errors;

    if (!Json::parseFromStream(builder, input, &root, &errors)) {
        throw std::runtime_error(
            "Invalid translation JSON in " + path.string() + ": " + errors
        );
    }

    if (!root.isObject()) {
        throw std::runtime_error(
            "Translation file must contain a JSON object: " + path.string()
        );
    }

    FileTranslationProvider::TranslationMap translations;
    for (const auto& key : root.getMemberNames()) {
        const auto& value = root[key];
        if (!value.isString()) {
            throw std::runtime_error(
                "Translation value must be a string for key '" + key +
                "' in " + path.string()
            );
        }
        translations.emplace(key, value.asString());
    }

    return translations;
}

} // namespace

FileTranslationProvider::FileTranslationProvider(
    std::filesystem::path directory,
    std::string defaultLocale
)
    : directory_(std::move(directory)),
      defaultLocale_(std::move(defaultLocale)) {
    if (defaultLocale_.empty()) {
        throw std::invalid_argument("Default locale must not be empty");
    }

    if (!std::filesystem::is_directory(directory_)) {
        throw std::runtime_error(
            "Translation directory does not exist: " + directory_.string()
        );
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory_)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }

        const auto locale = entry.path().stem().string();
        if (locale.empty()) {
            continue;
        }

        catalog_.emplace(locale, loadTranslations(entry.path()));
    }

    if (catalog_.find(defaultLocale_) == catalog_.end()) {
        throw std::runtime_error(
            "Default locale translation file not found: " +
            (directory_ / (defaultLocale_ + ".json")).string()
        );
    }
}

std::string FileTranslationProvider::translate(
    const std::string& locale,
    const std::string& key
) const {
    const auto findValue = [&](const std::string& candidate)
        -> const std::string* {
        const auto localeIt = catalog_.find(candidate);
        if (localeIt == catalog_.end()) {
            return nullptr;
        }

        const auto valueIt = localeIt->second.find(key);
        if (valueIt == localeIt->second.end()) {
            return nullptr;
        }

        return &valueIt->second;
    };

    if (const auto* value = findValue(locale)) {
        return *value;
    }

    if (locale != defaultLocale_) {
        if (const auto* value = findValue(defaultLocale_)) {
            return *value;
        }
    }

    return key;
}

const std::filesystem::path& FileTranslationProvider::directory() const noexcept {
    return directory_;
}

const std::string& FileTranslationProvider::defaultLocale() const noexcept {
    return defaultLocale_;
}

const FileTranslationProvider::Catalog&
FileTranslationProvider::catalog() const noexcept {
    return catalog_;
}

std::vector<std::string> FileTranslationProvider::locales() const {
    std::vector<std::string> result;
    result.reserve(catalog_.size());
    for (const auto& [locale, translations] : catalog_) {
        (void)translations;
        result.push_back(locale);
    }
    return result;
}

std::vector<std::string> FileTranslationProvider::keys(
    const std::string& locale
) const {
    std::vector<std::string> result;
    const auto localeIt = catalog_.find(locale);
    if (localeIt == catalog_.end()) {
        return result;
    }

    result.reserve(localeIt->second.size());
    for (const auto& [key, value] : localeIt->second) {
        (void)value;
        result.push_back(key);
    }
    return result;
}

} // namespace drogular