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

class ParentComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<section>Parent</section>";
    }

    std::vector<std::shared_ptr<drogular::Component>> children() override {
        return {
            std::make_shared<ChildComponent>()
        };
    }

private:
    class ChildComponent final : public drogular::Component {
    public:
        std::string render(drogular::RenderContext&) override {
            return "<p>Child</p>";
        }
    };
};

TEST(TestingTests, RendersComponentTree) {
    ParentComponent component;
    drogular::RenderContext context;

    const auto html = drogular::test::renderComponentTree(component, context);

    EXPECT_TRUE(drogular::test::contains(html, "Parent"));
    EXPECT_TRUE(drogular::test::contains(html, "Child"));
}