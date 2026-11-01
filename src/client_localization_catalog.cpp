#include <drogular/client_localization_catalog.hpp>

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace drogular {
namespace {

std::string contentVersion(const std::string& content) {
    // FNV-1a is sufficient here: this is a deterministic cache version, not a
    // cryptographic integrity primitive.
    std::uint64_t hash = 14695981039346656037ull;
    for (const unsigned char value : content) {
        hash ^= value;
        hash *= 1099511628211ull;
    }

    std::ostringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(16) << hash;
    return stream.str();
}

std::string serializeTranslations(
    const FileTranslationProvider::TranslationMap& translations
) {
    std::string result;
    for (const auto& [key, value] : translations) {
        result.append(key);
        result.push_back('\0');
        result.append(value);
        result.push_back('\0');
    }
    return result;
}

std::string normalizedBasePath(std::string value) {
    if (value.empty() || value.front() != '/') {
        throw std::invalid_argument(
            "Localization resource base path must be an absolute path"
        );
    }

    while (value.size() > 1 && value.back() == '/') {
        value.pop_back();
    }
    return value;
}

} // namespace

Json::Value ClientLocalizationManifest::toJson() const {
    Json::Value root(Json::objectValue);
    root["defaultLocale"] = defaultLocale;
    root["version"] = version;

    Json::Value localeNodes(Json::objectValue);
    for (const auto& [locale, descriptor] : locales) {
        Json::Value node(Json::objectValue);
        node["url"] = descriptor.url;
        node["version"] = descriptor.version;
        localeNodes[locale] = std::move(node);
    }
    root["locales"] = std::move(localeNodes);
    return root;
}

Json::Value ClientLocalizationResource::toJson() const {
    Json::Value root(Json::objectValue);
    root["locale"] = locale;
    root["version"] = version;

    Json::Value values(Json::objectValue);
    for (const auto& [key, value] : translations) {
        values[key] = value;
    }
    root["translations"] = std::move(values);
    return root;
}

ClientLocalizationCatalog::ClientLocalizationCatalog(
    Catalog catalog,
    std::string defaultLocale,
    std::string resourceBasePath
)
    : catalog_(std::move(catalog)),
      defaultLocale_(std::move(defaultLocale)),
      resourceBasePath_(normalizedBasePath(std::move(resourceBasePath))) {
    if (defaultLocale_.empty()) {
        throw std::invalid_argument("Default locale must not be empty");
    }
    if (catalog_.find(defaultLocale_) == catalog_.end()) {
        throw std::invalid_argument(
            "Default locale must exist in the client localization catalog"
        );
    }

    std::string manifestContent;
    for (const auto& [locale, translations] : catalog_) {
        const auto localeVersion = contentVersion(
            serializeTranslations(translations)
        );
        localeVersions_.emplace(locale, localeVersion);
        manifestContent.append(locale);
        manifestContent.push_back('\0');
        manifestContent.append(localeVersion);
        manifestContent.push_back('\0');
    }
    version_ = contentVersion(manifestContent);
}

const std::string& ClientLocalizationCatalog::defaultLocale() const noexcept {
    return defaultLocale_;
}

const std::string& ClientLocalizationCatalog::resourceBasePath() const noexcept {
    return resourceBasePath_;
}

const std::string& ClientLocalizationCatalog::version() const noexcept {
    return version_;
}

ClientLocalizationManifest ClientLocalizationCatalog::manifest() const {
    ClientLocalizationManifest result;
    result.defaultLocale = defaultLocale_;
    result.version = version_;

    for (const auto& [locale, translations] : catalog_) {
        (void)translations;
        result.locales.emplace(
            locale,
            ClientLocalizationLocale{
                resourceBasePath_ + "/" + locale + ".json",
                localeVersions_.at(locale)
            }
        );
    }
    return result;
}

ClientLocalizationResource ClientLocalizationCatalog::resource(
    const std::string& locale
) const {
    const auto it = catalog_.find(locale);
    if (it == catalog_.end()) {
        throw std::out_of_range("Unknown localization locale: " + locale);
    }

    return ClientLocalizationResource{
        locale,
        localeVersions_.at(locale),
        it->second
    };
}

bool ClientLocalizationCatalog::hasLocale(const std::string& locale) const noexcept {
    return catalog_.find(locale) != catalog_.end();
}

} // namespace drogular