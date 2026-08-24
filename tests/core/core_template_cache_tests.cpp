#include <drogular/template_cache.hpp>

#include <gtest/gtest.h>

#include <thread>
#include <vector>

using namespace drogular::template_compiler;

TEST(CoreTemplateCacheTests, CompilesTemplate) {
    TemplateCache cache;

    const auto compiled =
        cache.getOrCompile("<h1>{{ title }}</h1>");

    ASSERT_NE(compiled, nullptr);
    EXPECT_TRUE(cache.contains("<h1>{{ title }}</h1>"));
}

TEST(CoreTemplateCacheTests, ReturnsSameCompiledTemplateForSameHtml) {
    TemplateCache cache;

    const auto first =
        cache.getOrCompile("<h1>{{ title }}</h1>");

    const auto second =
        cache.getOrCompile("<h1>{{ title }}</h1>");

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}

TEST(CoreTemplateCacheTests, ReturnsDifferentCompiledTemplatesForDifferentHtml) {
    TemplateCache cache;

    const auto first =
        cache.getOrCompile("<h1>{{ title }}</h1>");

    const auto second =
        cache.getOrCompile("<p>{{ title }}</p>");

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(first.get(), second.get());
}

TEST(CoreTemplateCacheTests, ClearsTemplates) {
    TemplateCache cache;

    cache.getOrCompile("<h1>{{ title }}</h1>");

    ASSERT_TRUE(cache.contains("<h1>{{ title }}</h1>"));

    cache.clear();

    EXPECT_FALSE(cache.contains("<h1>{{ title }}</h1>"));
}

TEST(CoreTemplateCacheTests, ConcurrentGetOrCompileReturnsSingleCachedInstance) {
    TemplateCache cache;
    constexpr auto html = "<h1>{{ title }}</h1>";
    constexpr std::size_t workerCount = 16;

    std::vector<std::shared_ptr<CompiledTemplate>> results(workerCount);
    std::vector<std::thread> workers;
    workers.reserve(workerCount);

    for (std::size_t index = 0; index < workerCount; ++index) {
        workers.emplace_back([&, index] {
            results[index] = cache.getOrCompile(html);
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    ASSERT_NE(results.front(), nullptr);
    for (const auto& result : results) {
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result.get(), results.front().get());
    }
}
