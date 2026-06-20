#include <drogular/template_source_cache.hpp>

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

TEST(CoreTemplateSourceCacheTests, LoadsTemplate) {
    const auto path =
        writeTemplateFile(
            "drogular_template_source_cache_load.html",
            "<h1>Hello</h1>"
        );

    drogular::TemplateSourceCache cache;

    EXPECT_EQ(
        cache.load(path.string()),
        "<h1>Hello</h1>"
    );

    std::filesystem::remove(path);
}

TEST(CoreTemplateSourceCacheTests, CachesTemplateContent) {
    const auto path =
        writeTemplateFile(
            "drogular_template_source_cache_cached.html",
            "<h1>First</h1>"
        );

    drogular::TemplateSourceCache cache;

    EXPECT_EQ(
        cache.load(path.string()),
        "<h1>First</h1>"
    );

    {
        std::ofstream file(path);
        file << "<h1>Second</h1>";
    }

    EXPECT_EQ(
        cache.load(path.string()),
        "<h1>First</h1>"
    );

    std::filesystem::remove(path);
}

TEST(CoreTemplateSourceCacheTests, ClearsCache) {
    const auto path =
        writeTemplateFile(
            "drogular_template_source_cache_clear.html",
            "<h1>First</h1>"
        );

    drogular::TemplateSourceCache cache;

    EXPECT_EQ(
        cache.load(path.string()),
        "<h1>First</h1>"
    );

    {
        std::ofstream file(path);
        file << "<h1>Second</h1>";
    }

    cache.clear();

    EXPECT_EQ(
        cache.load(path.string()),
        "<h1>Second</h1>"
    );

    std::filesystem::remove(path);
}

TEST(CoreTemplateSourceCacheTests, ReportsCachedTemplate) {
    const auto path =
        writeTemplateFile(
            "drogular_template_source_cache_contains.html",
            "<h1>Hello</h1>"
        );

    drogular::TemplateSourceCache cache;

    EXPECT_FALSE(
        cache.contains(path.string())
    );

    cache.load(path.string());

    EXPECT_TRUE(
        cache.contains(path.string())
    );

    std::filesystem::remove(path);
}