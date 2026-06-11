#include <drogular/component_renderer.hpp>
#include <drogular/component_registry.hpp>
#include <drogular/component.hpp>

#include <gtest/gtest.h>

#include <json/json.h>

class CardComponent final : public drogular::Component {
public:
    static constexpr auto tag = "Card";

    std::string render(drogular::RenderContext&) override {
        return "<article>Card</article>";
    }
};

TEST(ComponentRendererTests, RendersSelfClosingComponentTag) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CardComponent>();

    drogular::RenderContext context;

    const auto html =
        drogular::component_renderer::render(
            "<div><Card /></div>",
            registry,
            context
        );

    EXPECT_EQ(
        html,
        "<div><article>Card</article></div>"
    );
}

TEST(ComponentRendererTests, LeavesUnknownComponentTagAsText) {
    drogular::ComponentRegistry registry;
    drogular::RenderContext context;

    const auto html =
        drogular::component_renderer::render(
            "<div><Unknown /></div>",
            registry,
            context
        );

    EXPECT_EQ(
        html,
        "<div><Unknown /></div>"
    );
}

class CardWithTitleComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CardWithTitle";

    std::string templateHtml() const override {
        return "<article><h2>{{ title }}</h2><p>{{ subtitle }}</p></article>";
    }
};

TEST(ComponentRendererTests, PassesStringAttributesAsInputs) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CardWithTitleComponent>();

    drogular::RenderContext context;

    const auto html =
        drogular::component_renderer::render(
            R"(<CardWithTitle title="Welcome" subtitle="Hello" />)",
            registry,
            context
        );

    EXPECT_EQ(
        html,
        "<article><h2>Welcome</h2><p>Hello</p></article>"
    );
}

class CardWithBoundTitleComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "BoundCard";

    std::string templateHtml() const override {
        return "<article>{{ title }}</article>";
    }
};

TEST(ComponentRendererTests, RendersAttributeBindings) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CardWithBoundTitleComponent>();

    drogular::RenderContext context;

    Json::Value user;
    user["name"] = "Vadim";

    context.set("user", user);

    const auto html =
        drogular::component_renderer::render(
            R"(<BoundCard title="{{ user.name }}" />)",
            registry,
            context
        );

    EXPECT_EQ(
        html,
        "<article>Vadim</article>"
    );
}

class CardWithSlotComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CardWithSlot";

    std::string templateHtml() const override {
        return "<article><h2>{{ title }}</h2><slot/></article>";
    }
};

TEST(ComponentRendererTests, RendersComponentWithDefaultSlot) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CardWithSlotComponent>();

    drogular::RenderContext context;

    const auto html =
        drogular::component_renderer::render(
            R"(<CardWithSlot title="Welcome"><p>Hello</p></CardWithSlot>)",
            registry,
            context
        );

    EXPECT_EQ(
        html,
        "<article><h2>Welcome</h2><p>Hello</p></article>"
    );
}