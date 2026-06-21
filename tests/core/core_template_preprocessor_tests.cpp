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

TEST(CoreTemplatePreprocessorTests, ProcessesNestedIncludes) {
    const auto root =
        std::filesystem::temp_directory_path();

    const auto nav =
        writeTemplateFile(
            "drogular_nested_nav.html",
            "<nav>Nav</nav>"
        );

    const auto header =
        writeTemplateFile(
            "drogular_nested_header.html",
            R"(@include("drogular_nested_nav.html")
<header>Header</header>)"
        );

    auto preprocessor = drogular::TemplatePreprocessor{
        drogular::TemplateLoader(root)
    };

    const auto result =
        preprocessor.process(
            R"(@include("drogular_nested_header.html")
<main>Content</main>)"
        );

    EXPECT_NE(result.find("<nav>Nav</nav>"), std::string::npos);
    EXPECT_NE(result.find("<header>Header</header>"), std::string::npos);
    EXPECT_NE(result.find("<main>Content</main>"), std::string::npos);

    std::filesystem::remove(nav);
    std::filesystem::remove(header);
}

TEST(CoreTemplatePreprocessorTests, ThrowsWhenIncludeDepthIsExceeded) {
    const auto first =
        writeTemplateFile(
            "drogular_cycle_first.html",
            R"(@include("drogular_cycle_second.html"))"
        );

    const auto second =
        writeTemplateFile(
            "drogular_cycle_second.html",
            R"(@include("drogular_cycle_first.html"))"
        );

    drogular::TemplatePreprocessor preprocessor{
        drogular::TemplateLoader(
            std::filesystem::temp_directory_path()
        )
    };

    EXPECT_THROW(
        preprocessor.process(
            R"(@include("drogular_cycle_first.html"))"
        ),
        std::runtime_error
    );

    std::filesystem::remove(first);
    std::filesystem::remove(second);
}