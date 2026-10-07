#include <drogular/translation_catalog_validator.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>
#include <string>

namespace {

using Catalog = drogular::TranslationCatalogValidator::Catalog;

TEST(TranslationCatalogValidatorTests, AcceptsCompleteCatalog) {
    const Catalog catalog{
        {"de", {{"common.cancel", "Abbrechen"}, {"common.save", "Speichern"}}},
        {"en", {{"common.cancel", "Cancel"}, {"common.save", "Save"}}}
    };

    const auto result = drogular::TranslationCatalogValidator::validate(catalog, "en");

    EXPECT_TRUE(result.valid());
    ASSERT_EQ(result.locales.size(), 1u);
    EXPECT_TRUE(result.locales.at("de").valid());
}

TEST(TranslationCatalogValidatorTests, ReportsMissingKeys) {
    const Catalog catalog{
        {"de", {{"common.save", "Speichern"}}},
        {"en", {{"common.cancel", "Cancel"}, {"common.save", "Save"}}}
    };

    const auto result = drogular::TranslationCatalogValidator::validate(catalog, "en");

    EXPECT_FALSE(result.valid());
    ASSERT_EQ(result.locales.at("de").missingKeys.size(), 1u);
    EXPECT_EQ(result.locales.at("de").missingKeys.front(), "common.cancel");
    EXPECT_TRUE(result.locales.at("de").extraKeys.empty());
}

TEST(TranslationCatalogValidatorTests, ReportsExtraKeys) {
    const Catalog catalog{
        {"de", {{"common.cancel", "Abbrechen"}, {"legacy.title", "Alt"}}},
        {"en", {{"common.cancel", "Cancel"}}}
    };

    const auto result = drogular::TranslationCatalogValidator::validate(catalog, "en");

    EXPECT_FALSE(result.valid());
    EXPECT_TRUE(result.locales.at("de").missingKeys.empty());
    ASSERT_EQ(result.locales.at("de").extraKeys.size(), 1u);
    EXPECT_EQ(result.locales.at("de").extraKeys.front(), "legacy.title");
}

TEST(TranslationCatalogValidatorTests, ReportsEachLocaleIndependently) {
    const Catalog catalog{
        {"de", {{"a", "A"}}},
        {"en", {{"a", "A"}, {"b", "B"}}},
        {"fr", {{"a", "A"}, {"b", "B"}, {"c", "C"}}}
    };

    const auto result = drogular::TranslationCatalogValidator::validate(catalog, "en");

    ASSERT_EQ(result.locales.size(), 2u);
    EXPECT_EQ(result.locales.at("de").missingKeys, std::vector<std::string>{"b"});
    EXPECT_EQ(result.locales.at("fr").extraKeys, std::vector<std::string>{"c"});
}

TEST(TranslationCatalogValidatorTests, RejectsMissingReferenceLocale) {
    const Catalog catalog{{"de", {{"common.save", "Speichern"}}}};

    EXPECT_THROW(
        drogular::TranslationCatalogValidator::validate(catalog, "en"),
        std::invalid_argument
    );
}

TEST(TranslationCatalogValidatorTests, ValidatesFileProviderUsingItsDefaultLocale) {
    const auto directory = std::filesystem::path(DROGULAR_SOURCE_DIR)
        / "examples/portal_demo/resources/localization";
    const drogular::FileTranslationProvider provider(directory, "en");

    const auto result = drogular::TranslationCatalogValidator::validate(provider);

    EXPECT_TRUE(result.valid());
    ASSERT_TRUE(result.locales.contains("de"));
    EXPECT_TRUE(result.locales.at("de").missingKeys.empty());
    EXPECT_TRUE(result.locales.at("de").extraKeys.empty());
}

} // namespace