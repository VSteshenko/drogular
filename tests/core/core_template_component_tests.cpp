#include <drogular/component.hpp>
#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <string>
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

class CoreTemplateComponentExternalTemplateComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag =
        "CoreTemplateComponentExternalTemplate";

    std::string templatePath() const override {
        return "drogular_component_external.html";
    }
};

TEST(CoreTemplateComponentTests, UsesTemplatePathWhenProvided) {
    const auto path =
        writeTemplateFile(
            "drogular_component_external.html",
            "<span>{{ title }}</span>"
        );

    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    options.setTemplateRoot(
        std::filesystem::temp_directory_path()
    );

    services.setOptions(&options);

    drogular::RenderContext context;
    context.setServices(&services);
    context.set(
        "title",
        std::string("External Component")
    );

    CoreTemplateComponentExternalTemplateComponent component;

    const auto html =
        component.render(context);

    EXPECT_TRUE(
        drogular::test::contains(
            html,
            "<span>External Component</span>"
        )
    );

    std::filesystem::remove(path);
}