#include <drogular/template_loader.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path writeTemplateFile(
    const std::string& name,
    const std::string& content
) {
    const auto path =
        std::filesystem::temp_directory_path() /
        name;

    std::ofstream file(path);
    file << content;

    return path;
}

} // namespace

TEST(CoreTemplateLoaderTests, LoadsTemplateFile) {
    const auto path =
        writeTemplateFile(
            "drogular_template_loader_test.html",
            "<h1>Hello</h1>"
        );

    drogular::TemplateLoader loader;

    EXPECT_EQ(
        loader.load(path.string()),
        "<h1>Hello</h1>"
    );

    std::filesystem::remove(path);
}

TEST(CoreTemplateLoaderTests, ThrowsWhenTemplateFileIsMissing) {
    drogular::TemplateLoader loader;

    EXPECT_THROW(
        loader.load(
            "/tmp/drogular_missing_template_file.html"
        ),
        std::runtime_error
    );
}

TEST(CoreTemplateLoaderTests, LoadsUtf8TemplateFile) {
    const auto path =
        writeTemplateFile(
            "drogular_template_loader_utf8_test.html",
            "<p>Привіт Drogular</p>"
        );

    drogular::TemplateLoader loader;

    EXPECT_EQ(
        loader.load(path.string()),
        "<p>Привіт Drogular</p>"
    );

    std::filesystem::remove(path);
}