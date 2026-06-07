#include <drogular/page.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <string>

class TestingPage final : public drogular::Page {
public:
    void onInit(drogular::RenderContext& context) override {
        context.set("title", std::string("Testing Page"));
    }

    std::string render(drogular::RenderContext& context) override {
        const auto title = context.require<std::string>("title");
        return "<h1>" + title + "</h1>";
    }
};

TEST(TestingTests, RendersPageWithLifecycle) {
    const auto result = drogular::test::renderPage<TestingPage>();

    EXPECT_EQ(result.html, "<h1>Testing Page</h1>");
    EXPECT_TRUE(result.context.contains("title"));
}
