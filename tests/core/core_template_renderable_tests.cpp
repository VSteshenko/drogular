#include <drogular/component.hpp>
#include <drogular/page.hpp>
#include <drogular/render_context.hpp>
#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

class SharedPipelinePage final : public drogular::TemplatePage {
public:
    std::string templateHtml() const override {
        return "<h1>{{ title }}</h1>";
    }
};

class SharedPipelineComponent final
    : public drogular::TemplateComponent
{
public:
    std::string templateHtml() const override {
        return "<h1>{{ title }}</h1>";
    }
};

class NestedLeafComponent final : public drogular::Component {
public:
    static constexpr auto tag = "NestedLeaf";

    std::string render(drogular::RenderContext&) override {
        return "<span>leaf</span>";
    }
};

class NestedTemplatePage final : public drogular::TemplatePage {
public:
    std::string templateHtml() const override {
        return "<main><NestedLeaf /></main>";
    }
};

class NestedTemplateComponent final
    : public drogular::TemplateComponent
{
public:
    std::string templateHtml() const override {
        return "<main><NestedLeaf /></main>";
    }
};

class LayoutTemplateComponent final
    : public drogular::TemplateComponent
{
public:
    static inline std::string layout;

    std::string templateHtml() const override {
        return "<p>{{ value }}</p>";
    }

    std::string layoutPath() const override {
        return layout;
    }
};

std::filesystem::path temporaryTemplatePath(
    const std::string& name
) {
    return std::filesystem::temp_directory_path() /
        ("drogular_" + name + ".html");
}

void writeFile(
    const std::filesystem::path& path,
    const std::string& content
) {
    std::ofstream output(path);
    output << content;
}

} // namespace

TEST(CoreTemplateRenderableTests, PageAndComponentUseSameExpressionPipeline) {
    drogular::RenderContext pageContext;
    pageContext.set("title", std::string("Shared"));

    drogular::RenderContext componentContext;
    componentContext.set("title", std::string("Shared"));

    SharedPipelinePage page;
    SharedPipelineComponent component;

    EXPECT_EQ(page.render(pageContext), "<h1>Shared</h1>");
    EXPECT_EQ(component.render(componentContext), "<h1>Shared</h1>");
}

TEST(CoreTemplateRenderableTests, PageAndComponentExpandNestedComponents) {
    drogular::ApplicationServices services;
    services.components().registerComponent<NestedLeafComponent>();

    drogular::RenderContext pageContext;
    pageContext.setServices(&services);

    drogular::RenderContext componentContext;
    componentContext.setServices(&services);

    NestedTemplatePage page;
    NestedTemplateComponent component;

    EXPECT_EQ(
        page.render(pageContext),
        "<main><span>leaf</span></main>"
    );
    EXPECT_EQ(
        component.render(componentContext),
        "<main><span>leaf</span></main>"
    );
}

TEST(CoreTemplateRenderableTests, ComponentUsesSharedLayoutPipeline) {
    const auto path = temporaryTemplatePath("component_layout");
    writeFile(path, "<article>@content</article>");

    LayoutTemplateComponent::layout = path.string();

    drogular::RenderContext context;
    context.set("value", std::string("content"));

    LayoutTemplateComponent component;

    EXPECT_EQ(
        component.render(context),
        "<article><p>content</p></article>"
    );

    std::filesystem::remove(path);
}