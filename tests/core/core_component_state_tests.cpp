#include <drogular/component.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/services.hpp>
#include <drogular/state.hpp>

#include <gtest/gtest.h>

class CoreCounterStateComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "CoreCounterState";

    void onInit(
        drogular::RenderContext& context
    ) override {
        context.set(
            "count",
            counter_.value()
        );
    }

    std::string templateHtml() const override {
        return "<p>{{ count }}</p>";
    }

private:
    drogular::State<int> counter_{42};
};

TEST(CoreComponentStateTests, RendersStateValue) {
    drogular::ApplicationServices services;

    services.components()
        .registerComponent<CoreCounterStateComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
        drogular::component_renderer::render(
            "<CoreCounterState />",
            services.components(),
            context
        );

    EXPECT_EQ(
        html,
        "<p>42</p>"
    );
}

class CoreMutableCounterStateComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "CoreMutableCounterState";

    void onInit(
        drogular::RenderContext& context
    ) override {
        counter_.set(5);

        context.set(
            "count",
            counter_.value()
        );
    }

    std::string templateHtml() const override {
        return "<p>{{ count }}</p>";
    }

private:
    drogular::State<int> counter_{0};
};

TEST(CoreComponentStateTests, StateCanBeModifiedInLifecycle) {
    drogular::ApplicationServices services;

    services.components()
        .registerComponent<CoreMutableCounterStateComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
        drogular::component_renderer::render(
            "<CoreMutableCounterState />",
            services.components(),
            context
        );

    EXPECT_EQ(
        html,
        "<p>5</p>"
    );
}