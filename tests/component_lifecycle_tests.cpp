#include <drogular/component.hpp>
#include <drogular/component_registry.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/services.hpp>
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

class LifecycleChildComponent final : public drogular::Component {
public:
    static constexpr auto tag = "LifecycleChild";

    static inline std::vector<std::string>* events = nullptr;

    void onInit(drogular::RenderContext&) override {
        events->push_back("child-init");
    }

    std::string render(drogular::RenderContext&) override {
        events->push_back("child-render");
        return "<span>Child</span>";
    }

    void onDestroy(drogular::RenderContext&) override {
        events->push_back("child-destroy");
    }
};

class LifecycleParentComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "LifecycleParent";

    static inline std::vector<std::string>* events = nullptr;

    void onInit(drogular::RenderContext&) override {
        events->push_back("parent-init");
    }

    std::string templateHtml() const override {
        events->push_back("parent-render");
        return "<section><LifecycleChild /></section>";
    }

    void onDestroy(drogular::RenderContext&) override {
        events->push_back("parent-destroy");
    }
};

TEST(ComponentLifecycleTests, CallsLifecycleHooksInOrderForNestedComponents) {
    std::vector<std::string> events;

    LifecycleParentComponent::events = &events;
    LifecycleChildComponent::events = &events;

    drogular::ComponentRegistry registry;
    registry.registerComponent<LifecycleParentComponent>();
    registry.registerComponent<LifecycleChildComponent>();

    drogular::ApplicationServices services;
    services.components().registerComponent<LifecycleParentComponent>();
    services.components().registerComponent<LifecycleChildComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
    drogular::component_renderer::render(
        "<LifecycleParent />",
        services.components(),
        context
    );

    EXPECT_EQ(
        html,
        "<section><span>Child</span></section>"
    );

    ASSERT_EQ(events.size(), 6);

    EXPECT_EQ(events[0], "parent-init");
    EXPECT_EQ(events[1], "parent-render");
    EXPECT_EQ(events[2], "child-init");
    EXPECT_EQ(events[3], "child-render");
    EXPECT_EQ(events[4], "child-destroy");
    EXPECT_EQ(events[5], "parent-destroy");
}