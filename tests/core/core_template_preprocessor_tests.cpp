#include <drogular/template_preprocessor.hpp>

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

TEST(CoreTemplatePreprocessorTests, IncludesPartial) {
    const auto partial =
        writeTemplateFile(
            "drogular_partial_header.html",
            "<header>Header</header>"
        );

    drogular::TemplateLoader loader(
        std::filesystem::temp_directory_path()
    );

    drogular::TemplatePreprocessor preprocessor(
        loader
    );

    const auto result =
        preprocessor.process(
            R"(@include("drogular_partial_header.html")
<main>Content</main>)"
        );

    EXPECT_NE(
        result.find("<header>Header</header>"),
        std::string::npos
    );

    EXPECT_NE(
        result.find("<main>Content</main>"),
        std::string::npos
    );

    std::filesystem::remove(partial);
}