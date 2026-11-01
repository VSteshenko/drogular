#include <drogular/client_localization_catalog.hpp>

#include <gtest/gtest.h>

TEST(ClientLocalizationCatalogTests, BuildsManifestAndLocaleResources) {
    drogular::ClientLocalizationCatalog::Catalog source{
        {"de", {{"app.title", "Anwendung"}}},
        {"en", {{"app.title", "App"}, {"common.save", "Save"}}}
    };

    const drogular::ClientLocalizationCatalog catalog(source, "en");
    const auto manifest = catalog.manifest();

    EXPECT_EQ(manifest.defaultLocale, "en");
    EXPECT_EQ(manifest.version, catalog.version());
    ASSERT_EQ(manifest.locales.size(), 2u);
    EXPECT_EQ(manifest.locales.at("de").url, "/assets/localization/de.json");
    EXPECT_EQ(manifest.locales.at("en").url, "/assets/localization/en.json");
    EXPECT_FALSE(manifest.locales.at("de").version.empty());

    const auto resource = catalog.resource("de");
    EXPECT_EQ(resource.locale, "de");
    EXPECT_EQ(resource.version, manifest.locales.at("de").version);
    EXPECT_EQ(resource.translations.at("app.title"), "Anwendung");
}

TEST(ClientLocalizationCatalogTests, ProducesDeterministicContentVersions) {
    drogular::ClientLocalizationCatalog::Catalog source{
        {"de", {{"app.title", "Anwendung"}}},
        {"en", {{"app.title", "App"}}}
    };

    const drogular::ClientLocalizationCatalog first(source, "en");
    const drogular::ClientLocalizationCatalog second(source, "en");

    EXPECT_EQ(first.version(), second.version());
    EXPECT_EQ(
        first.manifest().locales.at("de").version,
        second.manifest().locales.at("de").version
    );
}

TEST(ClientLocalizationCatalogTests, VersionsLocalesIndependently) {
    drogular::ClientLocalizationCatalog::Catalog before{
        {"de", {{"app.title", "Anwendung"}}},
        {"en", {{"app.title", "App"}}}
    };
    auto after = before;
    after["de"]["app.title"] = "Neue Anwendung";

    const drogular::ClientLocalizationCatalog first(before, "en");
    const drogular::ClientLocalizationCatalog second(after, "en");

    EXPECT_NE(first.version(), second.version());
    EXPECT_NE(
        first.manifest().locales.at("de").version,
        second.manifest().locales.at("de").version
    );
    EXPECT_EQ(
        first.manifest().locales.at("en").version,
        second.manifest().locales.at("en").version
    );
}

TEST(ClientLocalizationCatalogTests, SerializesStableJsonContract) {
    drogular::ClientLocalizationCatalog::Catalog source{
        {"en", {{"app.title", "App"}}}
    };
    const drogular::ClientLocalizationCatalog catalog(source, "en", "/i18n/");

    const auto manifest = catalog.manifest().toJson();
    EXPECT_EQ(manifest["defaultLocale"].asString(), "en");
    EXPECT_EQ(manifest["version"].asString(), catalog.version());
    EXPECT_EQ(manifest["locales"]["en"]["url"].asString(), "/i18n/en.json");

    const auto resource = catalog.resource("en").toJson();
    EXPECT_EQ(resource["locale"].asString(), "en");
    EXPECT_EQ(resource["translations"]["app.title"].asString(), "App");
}

TEST(ClientLocalizationCatalogTests, RejectsInvalidContractConfiguration) {
    drogular::ClientLocalizationCatalog::Catalog source{
        {"de", {{"app.title", "Anwendung"}}}
    };

    EXPECT_THROW(
        drogular::ClientLocalizationCatalog(source, "en"),
        std::invalid_argument
    );
    EXPECT_THROW(
        drogular::ClientLocalizationCatalog(source, "de", "relative/i18n"),
        std::invalid_argument
    );
}

TEST(ClientLocalizationCatalogTests, RejectsUnknownLocaleResource) {
    drogular::ClientLocalizationCatalog::Catalog source{
        {"en", {{"app.title", "App"}}}
    };
    const drogular::ClientLocalizationCatalog catalog(source, "en");

    EXPECT_FALSE(catalog.hasLocale("de"));
    EXPECT_THROW(catalog.resource("de"), std::out_of_range);
}