#include <drogular/component.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/services.hpp>
#include <drogular/state.hpp>

#include <gtest/gtest.h>

using SharedCounterState =
    drogular::State<int>;

class SharedCounterComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "SharedCounter";

    void onInit(
        drogular::RenderContext& context
    ) override {
        auto counter =
            context.requireService<SharedCounterState>();

        counter->set(
            counter->value() + 1
        );

        context.set(
            "count",
            counter->value()
        );
    }

    std::string templateHtml() const override {
        return "<p>{{ count }}</p>";
    }
};

TEST(SharedStateTests, ComponentsCanShareStateThroughServices) {
    drogular::ApplicationServices services;

    services.addFactory<SharedCounterState>(
        drogular::ServiceLifetime::Singleton,
        []() {
            return std::make_shared<SharedCounterState>(0);
        }
    );

    services.components()
        .registerComponent<SharedCounterComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
        drogular::component_renderer::render(
            "<SharedCounter /><SharedCounter />",
            services.components(),
            context
        );

    EXPECT_EQ(
        html,
        "<p>1</p><p>2</p>"
    );
}

TEST(SharedStateTests, SharedStatePersistsBetweenRenderCalls) {
    drogular::ApplicationServices services;

    services.addFactory<SharedCounterState>(
        drogular::ServiceLifetime::Singleton,
        []() {
            return std::make_shared<SharedCounterState>(0);
        }
    );

    services.components()
        .registerComponent<SharedCounterComponent>();

    drogular::RenderContext firstContext;
    firstContext.setServices(&services);

    const auto firstHtml =
        drogular::component_renderer::render(
            "<SharedCounter />",
            services.components(),
            firstContext
        );

    drogular::RenderContext secondContext;
    secondContext.setServices(&services);

    const auto secondHtml =
        drogular::component_renderer::render(
            "<SharedCounter />",
            services.components(),
            secondContext
        );

    EXPECT_EQ(firstHtml, "<p>1</p>");
    EXPECT_EQ(secondHtml, "<p>2</p>");
}