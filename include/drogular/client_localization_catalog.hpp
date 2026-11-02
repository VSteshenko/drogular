#pragma once

#include <drogular/file_translation_provider.hpp>

#include <json/json.h>

#include <map>
#include <string>

namespace drogular {

/**
 * Describes one locale resource exposed to a browser client.
 */
struct ClientLocalizationLocale {
    std::string url;
    std::string version;
};

/**
 * Browser-facing localization manifest.
 *
 * The manifest version changes whenever any locale resource changes. Each
 * locale also has an independent content version so clients can invalidate
 * only the resources whose translations changed.
 */
struct ClientLocalizationManifest {
    std::string defaultLocale;
    std::string version;
    std::map<std::string, ClientLocalizationLocale> locales;

    Json::Value toJson() const;
};

/**
 * Browser-facing translation resource for one locale.
 */
struct ClientLocalizationResource {
    std::string locale;
    std::string version;
    FileTranslationProvider::TranslationMap translations;

    Json::Value toJson() const;
};

/**
 * Builds deterministic browser localization resources from a translation
 * catalog. The source catalog remains the single source of truth.
 */
class ClientLocalizationCatalog {
public:
    using Catalog = FileTranslationProvider::Catalog;

    ClientLocalizationCatalog(
        Catalog catalog,
        std::string defaultLocale,
        std::string resourceBasePath = "/assets/localization",
        std::string resourceSuffix = ".json"
    );

    const std::string& defaultLocale() const noexcept;
    const std::string& resourceBasePath() const noexcept;
    const std::string& resourceSuffix() const noexcept;
    const std::string& version() const noexcept;

    ClientLocalizationManifest manifest() const;
    ClientLocalizationResource resource(const std::string& locale) const;
    bool hasLocale(const std::string& locale) const noexcept;

private:
    Catalog catalog_;
    std::string defaultLocale_;
    std::string resourceBasePath_;
    std::string resourceSuffix_;
    std::map<std::string, std::string> localeVersions_;
    std::string version_;
};

} // namespace drogular