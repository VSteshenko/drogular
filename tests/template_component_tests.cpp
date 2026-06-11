#include <drogular/component.hpp>
#include <drogular/services.hpp>

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

    EXPECT_EQ(component.render(context), "<h1>Hello TemplateComponent</h1>");
}

class ParamTemplateComponent final : public drogular::TemplateComponent {
public:
    std::string templateHtml() const override {
        return "<h2>{{ title }}</h2>";
    }
};

TEST(TemplateComponentTests, RendersComponentParams) {
    ParamTemplateComponent component;
    drogular::RenderContext context;

    component.setParam("title", std::string("Hello Card"));

    EXPECT_EQ(component.render(context), "<h2>Hello Card</h2>");
}

class InputTemplateComponent final : public drogular::TemplateComponent {
public:
    std::string templateHtml() const override {
        return "<article><h2>{{ title }}</h2><p>{{ subtitle }}</p></article>";
    }
};

TEST(TemplateComponentTests, RendersComponentInputs) {
    InputTemplateComponent component;
    drogular::RenderContext context;

    component.setInput("title", std::string("Card Title"));
    component.setInput("subtitle", std::string("Card Subtitle"));

    EXPECT_EQ(
        component.render(context),
        "<article><h2>Card Title</h2><p>Card Subtitle</p></article>"
    );
}

class TemplateComponentCard final : public drogular::Component {
public:
    static constexpr auto tag = "Card";

    std::string render(drogular::RenderContext&) override {
        return "<article>Card from TemplateComponent</article>";
    }
};

class ComponentTagTemplateComponent final : public drogular::TemplateComponent {
public:
    std::string templateHtml() const override {
        return "<section><Card /></section>";
    }
};

TEST(TemplateComponentTests, RendersComponentTags) {
    drogular::ApplicationServices services;
    services.components().registerComponent<TemplateComponentCard>();

    ComponentTagTemplateComponent component;
    drogular::RenderContext context;
    context.setServices(&services);

    EXPECT_EQ(
        component.render(context),
        "<section><article>Card from TemplateComponent</article></section>"
    );
}