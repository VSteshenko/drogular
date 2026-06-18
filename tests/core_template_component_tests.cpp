#include <drogular/component.hpp>
#include <drogular/services.hpp>

#include <gtest/gtest.h>

#include <string>

class CoreTestTemplateComponent final : public drogular::TemplateComponent {
public:
    void onInit(drogular::RenderContext& context) override {
        context.set("title", std::string("Hello TemplateComponent"));
    }

    std::string templateHtml() const override {
        return "<h1>{{ title }}</h1>";
    }
};

TEST(CoreTemplateComponentTests, RendersTemplateWithContext) {
    CoreTestTemplateComponent component;
    drogular::RenderContext context;

    component.onInit(context);

    EXPECT_EQ(component.render(context), "<h1>Hello TemplateComponent</h1>");
}

class CoreParamTemplateComponent final : public drogular::TemplateComponent {
public:
    std::string templateHtml() const override {
        return "<h2>{{ title }}</h2>";
    }
};

TEST(CoreTemplateComponentTests, RendersComponentParams) {
    CoreParamTemplateComponent component;
    drogular::RenderContext context;

    component.setParam("title", std::string("Hello Card"));

    EXPECT_EQ(component.render(context), "<h2>Hello Card</h2>");
}

class CoreInputTemplateComponent final : public drogular::TemplateComponent {
public:
    std::string templateHtml() const override {
        return "<article><h2>{{ title }}</h2><p>{{ subtitle }}</p></article>";
    }
};

TEST(CoreTemplateComponentTests, RendersComponentInputs) {
    CoreInputTemplateComponent component;
    drogular::RenderContext context;

    component.setInput("title", std::string("Card Title"));
    component.setInput("subtitle", std::string("Card Subtitle"));

    EXPECT_EQ(
        component.render(context),
        "<article><h2>Card Title</h2><p>Card Subtitle</p></article>"
    );
}

class CoreTemplateComponentCard final : public drogular::Component {
public:
    static constexpr auto tag = "CoreCard";

    std::string render(drogular::RenderContext&) override {
        return "<article>Card from TemplateComponent</article>";
    }
};

class CoreComponentTagTemplateComponent final : public drogular::TemplateComponent {
public:
    std::string templateHtml() const override {
        return "<section><CoreCard /></section>";
    }
};

TEST(CoreTemplateComponentTests, RendersComponentTags) {
    drogular::ApplicationServices services;
    services.components().registerComponent<CoreTemplateComponentCard>();

    CoreComponentTagTemplateComponent component;
    drogular::RenderContext context;
    context.setServices(&services);

    EXPECT_EQ(
        component.render(context),
        "<section><article>Card from TemplateComponent</article></section>"
    );
}