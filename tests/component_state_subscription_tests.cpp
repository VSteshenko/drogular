#include <drogular/component.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/services.hpp>
#include <drogular/state.hpp>

#include <gtest/gtest.h>

class SubscribedCounterComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "SubscribedCounter";

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

TEST(ComponentStateSubscriptionTests, ComponentCanSubscribeToStateChanges) {
    SubscribedCounterComponent::receivedValue = 0;

    drogular::ApplicationServices services;

    services.components()
        .registerComponent<SubscribedCounterComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
        drogular::component_renderer::render(
            "<SubscribedCounter />",
            services.components(),
            context
        );

    EXPECT_EQ(html, "<p>7</p>");
    EXPECT_EQ(SubscribedCounterComponent::receivedValue, 7);
}