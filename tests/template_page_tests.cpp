#include <drogular/page.hpp>
#include <drogular/services.hpp>

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

class TemplatePageCardComponent final : public drogular::Component {
public:
    static constexpr auto tag = "Card";

    std::string render(drogular::RenderContext&) override {
        return "<article>Card from TemplatePage</article>";
    }
};

class ComponentTagTemplatePage final : public drogular::TemplatePage {
public:
    std::string templateHtml() const override {
        return "<main><Card /></main>";
    }
};

TEST(TemplatePageTests, RendersComponentTags) {
    drogular::ApplicationServices services;
    services.components().registerComponent<TemplatePageCardComponent>();

    ComponentTagTemplatePage page;
    drogular::RenderContext context;
    context.setServices(&services);

    EXPECT_EQ(
        page.render(context),
        "<main><article>Card from TemplatePage</article></main>"
    );
}