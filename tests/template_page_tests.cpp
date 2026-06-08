#include <drogular/page.hpp>

#include <gtest/gtest.h>

#include <string>

class TestTemplatePage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        context.set("title", std::string("Hello TemplatePage"));
    }

    std::string templateHtml() const override {
        return "<h1>{{ title }}</h1>";
    }
};

TEST(TemplatePageTests, RendersTemplateWithContext) {
    TestTemplatePage page;
    drogular::RenderContext context;

    page.onInit(context);

    EXPECT_EQ(
        page.render(context),
        "<h1>Hello TemplatePage</h1>"
    );
}
