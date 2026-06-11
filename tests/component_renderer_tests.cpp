#include <drogular/component_renderer.hpp>
#include <drogular/component_registry.hpp>
#include <drogular/component.hpp>

#include <gtest/gtest.h>

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
