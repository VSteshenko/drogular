#include <drogular/component.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/services.hpp>
#include <drogular/state.hpp>

#include <gtest/gtest.h>

using CoreSharedCounterState =
    drogular::State<int>;

class CoreSharedCounterComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "CoreSharedCounter";

    void onInit(
        drogular::RenderContext& context
    ) override {
        auto counter =
            context.requireService<CoreSharedCounterState>();

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

TEST(CoreSharedStateTests, ComponentsCanShareStateThroughServices) {
    drogular::ApplicationServices services;

    services.addFactory<CoreSharedCounterState>(
        drogular::ServiceLifetime::Singleton,
        []() {
            return std::make_shared<CoreSharedCounterState>(0);
        }
    );

    services.components()
        .registerComponent<CoreSharedCounterComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
        drogular::component_renderer::render(
            "<CoreSharedCounter /><CoreSharedCounter />",
            services.components(),
            context
        );

    EXPECT_EQ(
        html,
        "<p>1</p><p>2</p>"
    );
}

TEST(CoreSharedStateTests, SharedStatePersistsBetweenRenderCalls) {
    drogular::ApplicationServices services;

    services.addFactory<CoreSharedCounterState>(
        drogular::ServiceLifetime::Singleton,
        []() {
            return std::make_shared<CoreSharedCounterState>(0);
        }
    );

    services.components()
        .registerComponent<CoreSharedCounterComponent>();

    drogular::RenderContext firstContext;
    firstContext.setServices(&services);

    const auto firstHtml =
        drogular::component_renderer::render(
            "<CoreSharedCounter />",
            services.components(),
            firstContext
        );

    drogular::RenderContext secondContext;
    secondContext.setServices(&services);

    const auto secondHtml =
        drogular::component_renderer::render(
            "<CoreSharedCounter />",
            services.components(),
            secondContext
        );

    EXPECT_EQ(firstHtml, "<p>1</p>");
    EXPECT_EQ(secondHtml, "<p>2</p>");
}