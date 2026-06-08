#include <drogular/component.hpp>

#include <gtest/gtest.h>

#include <string>

class TestTemplateComponent final : public drogular::TemplateComponent {
public:
    void onInit(drogular::RenderContext& context) override {
        context.set("title", std::string("Hello TemplateComponent"));
    }

    std::string templateHtml() const override {
        return "<h1>{{ title }}</h1>";
    }
};

TEST(TemplateComponentTests, RendersTemplateWithContext) {
    TestTemplateComponent component;
    drogular::RenderContext context;

    component.onInit(context);

    EXPECT_EQ(
        component.render(context),
        "<h1>Hello TemplateComponent</h1>"
    );
}
