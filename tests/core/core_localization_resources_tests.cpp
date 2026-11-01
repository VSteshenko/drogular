#include <drogular/localization_resources.hpp>

#include <gtest/gtest.h>

#include <string>

TEST(LocalizationResourcesTests, ExposesStableFrameworkAssetPath) {
    EXPECT_EQ(
        drogular::localization_resources::ScriptPath,
        "/__drogular/assets/localization.js"
    );
}

TEST(LocalizationResourcesTests, LoadsManifestAndVersionedLocaleResources) {
    const std::string script(drogular::localization_resources::script());

    EXPECT_NE(script.find("const loadManifest = async"), std::string::npos);
    EXPECT_NE(script.find("const loadResource = async"), std::string::npos);
    EXPECT_NE(
        script.find("cached?.version === descriptor.version"),
        std::string::npos
    );
    EXPECT_NE(
        script.find("resource.version !== descriptor.version"),
        std::string::npos
    );
}

TEST(LocalizationResourcesTests, UsesServerCompatibleFallbackOrder) {
    const std::string script(drogular::localization_resources::script());

    const auto requested = script.find("resources.get(locale)?.translations");
    const auto fallback = script.find(
        "resources.get(manifestValue.defaultLocale)?.translations"
    );
    const auto keyFallback = script.find("return key;", fallback);

    ASSERT_NE(requested, std::string::npos);
    ASSERT_NE(fallback, std::string::npos);
    ASSERT_NE(keyFallback, std::string::npos);
    EXPECT_LT(requested, fallback);
    EXPECT_LT(fallback, keyFallback);
}

TEST(LocalizationResourcesTests, FallsBackUnknownLocaleToDefaultLocale) {
    const std::string script(drogular::localization_resources::script());

    EXPECT_NE(
        script.find("manifest.locales[locale]\n            ? locale\n            : manifest.defaultLocale"),
        std::string::npos
    );
}

TEST(LocalizationResourcesTests, PublishesLocaleChanges) {
    const std::string script(drogular::localization_resources::script());

    EXPECT_NE(
        script.find("document.documentElement.lang = requested"),
        std::string::npos
    );
    EXPECT_NE(script.find("dg:locale-changed"), std::string::npos);
}

TEST(LocalizationResourcesTests, ExposesExplicitRuntimeApi) {
    const std::string script(drogular::localization_resources::script());

    EXPECT_NE(script.find("configure,"), std::string::npos);
    EXPECT_NE(script.find("ready,"), std::string::npos);
    EXPECT_NE(script.find("setLocale,"), std::string::npos);
    EXPECT_NE(script.find("translate,"), std::string::npos);
    EXPECT_NE(
        script.find("globalThis.drogularLocalization = api"),
        std::string::npos
    );
}

TEST(LocalizationResourcesTests, PersistsLocalizationResponsesForOfflineUse) {
    const std::string script(drogular::localization_resources::script());

    EXPECT_NE(
        script.find("const CACHE_NAME = 'drogular-localization-v1'"),
        std::string::npos
    );
    EXPECT_NE(script.find("await caches.open(CACHE_NAME)"), std::string::npos);
    EXPECT_NE(
        script.find("await cache.put(cacheKey, response.clone())"),
        std::string::npos
    );
    EXPECT_NE(
        script.find("const cached = await cache.match(cacheKey)"),
        std::string::npos
    );
}

TEST(LocalizationResourcesTests, UsesVersionedLocaleCacheKeys) {
    const std::string script(drogular::localization_resources::script());

    EXPECT_NE(
        script.find("value.searchParams.set('dg-l10n-version', version)"),
        std::string::npos
    );
    EXPECT_NE(
        script.find("versionedUrl(descriptor.url, descriptor.version)"),
        std::string::npos
    );
}

TEST(LocalizationResourcesTests, RetainsUnchangedLocaleResourcesAcrossManifestUpdates) {
    const std::string script(drogular::localization_resources::script());

    EXPECT_NE(
        script.find("next.locales[locale]?.version !== resource.version"),
        std::string::npos
    );
    const auto loadManifest = script.find("const loadManifest");
    const auto loadResource = script.find("const loadResource");
    ASSERT_NE(loadManifest, std::string::npos);
    ASSERT_NE(loadResource, std::string::npos);
    EXPECT_EQ(
        script.substr(loadManifest, loadResource - loadManifest)
            .find("resources.clear();"),
        std::string::npos
    );
}

TEST(LocalizationResourcesTests, PrunesSupersededPersistentLocaleVersions) {
    const std::string script(drogular::localization_resources::script());

    EXPECT_NE(
        script.find("const pruneLocalizationCache = async"),
        std::string::npos
    );
    EXPECT_NE(script.find("const keys = await cache.keys()"), std::string::npos);
    EXPECT_NE(
        script.find("keep.has(request.url) ? Promise.resolve(false) : cache.delete(request)"),
        std::string::npos
    );
}