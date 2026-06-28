#include <drogular/component.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/services.hpp>
#include <drogular/state.hpp>
#include <drogular/render_context.hpp>

#include <gtest/gtest.h>

class CoreSubscribedCounterComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "CoreSubscribedCounter";

    static inline int receivedValue = 0;

    void onInit(
        drogular::RenderContext& context
    ) override {
        counter_.subscribe(
            [](const int& value) {
                receivedValue = value;
            }
        );

        counter_.set(7);

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

TEST(CoreComponentStateSubscriptionTests, ComponentCanSubscribeToStateChanges) {
    CoreSubscribedCounterComponent::receivedValue = 0;

    drogular::ApplicationServices services;

    services.components()
        .registerComponent<CoreSubscribedCounterComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
        drogular::component_renderer::render(
            "<CoreSubscribedCounter />",
            services.components(),
            context
        );

    EXPECT_EQ(html, "<p>7</p>");
    EXPECT_EQ(CoreSubscribedCounterComponent::receivedValue, 7);
}