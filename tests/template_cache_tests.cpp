#include <drogular/template_cache.hpp>

#include <gtest/gtest.h>

using namespace drogular::template_compiler;

TEST(TemplateCacheTests, CompilesTemplate) {
    TemplateCache cache;

    const auto compiled =
        cache.getOrCompile("<h1>{{ title }}</h1>");

    ASSERT_NE(compiled, nullptr);
    EXPECT_TRUE(cache.contains("<h1>{{ title }}</h1>"));
}

TEST(TemplateCacheTests, ReturnsSameCompiledTemplateForSameHtml) {
    TemplateCache cache;

    const auto first =
        cache.getOrCompile("<h1>{{ title }}</h1>");

    const auto second =
        cache.getOrCompile("<h1>{{ title }}</h1>");

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}

TEST(TemplateCacheTests, ReturnsDifferentCompiledTemplatesForDifferentHtml) {
    TemplateCache cache;

    const auto first =
        cache.getOrCompile("<h1>{{ title }}</h1>");

    const auto second =
        cache.getOrCompile("<p>{{ title }}</p>");

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(first.get(), second.get());
}

TEST(TemplateCacheTests, ClearsTemplates) {
    TemplateCache cache;

    cache.getOrCompile("<h1>{{ title }}</h1>");

    ASSERT_TRUE(cache.contains("<h1>{{ title }}</h1>"));

    cache.clear();

    EXPECT_FALSE(cache.contains("<h1>{{ title }}</h1>"));
}