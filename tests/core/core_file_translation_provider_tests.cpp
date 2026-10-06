#include <drogular/file_translation_provider.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TranslationDirectory {
public:
    TranslationDirectory() {
        const auto suffix = std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()
        );
        path_ = std::filesystem::temp_directory_path() /
            ("drogular_translation_provider_" + suffix);
        std::filesystem::create_directories(path_);
    }

    ~TranslationDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }

    const std::filesystem::path& path() const { return path_; }

    void write(const std::string& name, const std::string& content) const {
        std::ofstream output(path_ / name);
        output << content;
    }

private:
    std::filesystem::path path_;
};

} // namespace

TEST(FileTranslationProviderTests, LoadsLocaleFiles) {
    TranslationDirectory directory;
    directory.write("en.json", R"({"app.title":"Test App"})");
    directory.write("de.json", R"({"app.title":"Test Anwendung"})");

    drogular::FileTranslationProvider provider(directory.path());

    EXPECT_EQ(provider.translate("en", "app.title"), "Test App");
    EXPECT_EQ(provider.translate("de", "app.title"), "Test Anwendung");
}

TEST(FileTranslationProviderTests, FallsBackToDefaultLocale) {
    TranslationDirectory directory;
    directory.write(
        "en.json",
        R"({"app.title":"Test App","common.save":"Save"})"
    );
    directory.write("de.json", R"({"app.title":"Test Anwendung"})");

    drogular::FileTranslationProvider provider(directory.path(), "en");

    EXPECT_EQ(provider.translate("de", "common.save"), "Save");
    EXPECT_EQ(provider.translate("fr", "app.title"), "Test App");
}

TEST(FileTranslationProviderTests, ReturnsKeyWhenTranslationIsMissing) {
    TranslationDirectory directory;
    directory.write("en.json", R"({"app.title":"Test App"})");

    drogular::FileTranslationProvider provider(directory.path());

    EXPECT_EQ(provider.translate("de", "missing.key"), "missing.key");
}

TEST(FileTranslationProviderTests, ExposesCatalogForValidationTools) {
    TranslationDirectory directory;
    directory.write(
        "en.json",
        R"({"app.title":"Test App","common.save":"Save"})"
    );
    directory.write("de.json", R"({"app.title":"Test Anwendung"})");

    drogular::FileTranslationProvider provider(directory.path());

    EXPECT_EQ(provider.defaultLocale(), "en");
    EXPECT_EQ(provider.locales(), (std::vector<std::string>{"de", "en"}));
    EXPECT_EQ(
        provider.keys("en"),
        (std::vector<std::string>{"app.title", "common.save"})
    );
    EXPECT_EQ(provider.catalog().at("de").at("app.title"), "Test Anwendung");
}

TEST(FileTranslationProviderTests, RejectsMissingDefaultLocale) {
    TranslationDirectory directory;
    directory.write("de.json", R"({"app.title":"Test Anwendung"})");

    EXPECT_THROW(
        drogular::FileTranslationProvider(directory.path(), "en"),
        std::runtime_error
    );
}

TEST(FileTranslationProviderTests, RejectsInvalidJson) {
    TranslationDirectory directory;
    directory.write("en.json", "{ invalid json }");

    EXPECT_THROW(
        drogular::FileTranslationProvider(directory.path()),
        std::runtime_error
    );
}

TEST(FileTranslationProviderTests, RejectsNonStringValues) {
    TranslationDirectory directory;
    directory.write("en.json", R"({"app.title":42})");

    EXPECT_THROW(
        drogular::FileTranslationProvider(directory.path()),
        std::runtime_error
    );
}