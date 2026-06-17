#include <drogular/component.hpp>
#include <drogular/component_registry.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

class LifecycleTestComponent final : public drogular::Component {
public:
    std::vector<std::string>* events = nullptr;

    void onInit(drogular::RenderContext&) override {
        events->push_back("init");
    }

    std::string render(drogular::RenderContext&) override {
        events->push_back("render");
        return "<p>Hello</p>";
    }

    void onDestroy(drogular::RenderContext&) override {
        events->push_back("destroy");
    }
};

TEST(ComponentLifecycleTests, CallsLifecycleHooksAroundRender) {
    std::vector<std::string> events;

    LifecycleTestComponent component;
    component.events = &events;

    drogular::RenderContext context;

    const auto html =
        drogular::test::renderComponentTree(
            component,
            context
        );

    EXPECT_EQ(html, "<p>Hello</p>");

    ASSERT_EQ(events.size(), 3);
    EXPECT_EQ(events[0], "init");
    EXPECT_EQ(events[1], "render");
    EXPECT_EQ(events[2], "destroy");
}

class LifecycleTagComponent final : public drogular::Component {
public:
    static constexpr auto tag = "LifecycleTag";

    static inline std::vector<std::string> events;

    void onInit(drogular::RenderContext&) override {
        events.push_back("init");
    }

    std::string render(drogular::RenderContext&) override {
        events.push_back("render");
        return "<span>Tag</span>";
    }

    void onDestroy(drogular::RenderContext&) override {
        events.push_back("destroy");
    }
};

TEST(ComponentLifecycleTests, CallsLifecycleHooksForComponentTags) {
    LifecycleTagComponent::events.clear();

    drogular::ComponentRegistry registry;
    registry.registerComponent<LifecycleTagComponent>();

    drogular::RenderContext context;

    const auto html =
        drogular::component_renderer::render(
            "<LifecycleTag />",
            registry,
            context
        );

    EXPECT_EQ(html, "<span>Tag</span>");

    ASSERT_EQ(LifecycleTagComponent::events.size(), 3);
    EXPECT_EQ(LifecycleTagComponent::events[0], "init");
    EXPECT_EQ(LifecycleTagComponent::events[1], "render");
    EXPECT_EQ(LifecycleTagComponent::events[2], "destroy");
}