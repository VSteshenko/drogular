#include <drogular/component.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/services.hpp>
#include <drogular/state.hpp>

#include <gtest/gtest.h>

class CounterStateComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "CounterState";

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

TEST(ComponentStateTests, RendersStateValue) {
    drogular::ApplicationServices services;

    services.components()
        .registerComponent<CounterStateComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
        drogular::component_renderer::render(
            "<CounterState />",
            services.components(),
            context
        );

    EXPECT_EQ(
        html,
        "<p>42</p>"
    );
}

class MutableCounterStateComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "MutableCounterState";

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

TEST(ComponentStateTests, StateCanBeModifiedInLifecycle) {
    drogular::ApplicationServices services;

    services.components()
        .registerComponent<MutableCounterStateComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
        drogular::component_renderer::render(
            "<MutableCounterState />",
            services.components(),
            context
        );

    EXPECT_EQ(
        html,
        "<p>5</p>"
    );
}