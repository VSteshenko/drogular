#include <drogular/component.hpp>
#include <drogular/component_registry.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

class CoreLifecycleTestComponent final : public drogular::Component {
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

TEST(CoreComponentLifecycleTests, CallsLifecycleHooksAroundRender) {
    std::vector<std::string> events;

    CoreLifecycleTestComponent component;
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

class CoreLifecycleTagComponent final : public drogular::Component {
public:
    static constexpr auto tag = "CoreLifecycleTag";

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

TEST(CoreComponentLifecycleTests, CallsLifecycleHooksForComponentTags) {
    CoreLifecycleTagComponent::events.clear();

    drogular::ComponentRegistry registry;
    registry.registerComponent<CoreLifecycleTagComponent>();

    drogular::RenderContext context;

    const auto html =
        drogular::component_renderer::render(
            "<CoreLifecycleTag />",
            registry,
            context
        );

    EXPECT_EQ(html, "<span>Tag</span>");

    ASSERT_EQ(CoreLifecycleTagComponent::events.size(), 3);
    EXPECT_EQ(CoreLifecycleTagComponent::events[0], "init");
    EXPECT_EQ(CoreLifecycleTagComponent::events[1], "render");
    EXPECT_EQ(CoreLifecycleTagComponent::events[2], "destroy");
}

class CoreLifecycleChildComponent final : public drogular::Component {
public:
    static constexpr auto tag = "CoreLifecycleChild";

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

class CoreLifecycleParentComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreLifecycleParent";

    static inline std::vector<std::string>* events = nullptr;

    void onInit(drogular::RenderContext&) override {
        events->push_back("parent-init");
    }

    std::string templateHtml() const override {
        events->push_back("parent-render");
        return "<section><CoreLifecycleChild /></section>";
    }

    void onDestroy(drogular::RenderContext&) override {
        events->push_back("parent-destroy");
    }
};

TEST(CoreComponentLifecycleTests, CallsLifecycleHooksInOrderForNestedComponents) {
    std::vector<std::string> events;

    CoreLifecycleParentComponent::events = &events;
    CoreLifecycleChildComponent::events = &events;

    drogular::ComponentRegistry registry;
    registry.registerComponent<CoreLifecycleParentComponent>();
    registry.registerComponent<CoreLifecycleChildComponent>();

    drogular::ApplicationServices services;
    services.components().registerComponent<CoreLifecycleParentComponent>();
    services.components().registerComponent<CoreLifecycleChildComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
    drogular::component_renderer::render(
        "<CoreLifecycleParent />",
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

class CoreLifecycleFirstChildComponent final : public drogular::Component {
public:
    static constexpr auto tag = "CoreLifecycleFirstChild";

    static inline std::vector<std::string>* events = nullptr;

    void onInit(drogular::RenderContext&) override {
        events->push_back("first-init");
    }

    std::string render(drogular::RenderContext&) override {
        events->push_back("first-render");
        return "<span>First</span>";
    }

    void onDestroy(drogular::RenderContext&) override {
        events->push_back("first-destroy");
    }
};

class CoreLifecycleSecondChildComponent final : public drogular::Component {
public:
    static constexpr auto tag = "CoreLifecycleSecondChild";

    static inline std::vector<std::string>* events = nullptr;

    void onInit(drogular::RenderContext&) override {
        events->push_back("second-init");
    }

    std::string render(drogular::RenderContext&) override {
        events->push_back("second-render");
        return "<span>Second</span>";
    }

    void onDestroy(drogular::RenderContext&) override {
        events->push_back("second-destroy");
    }
};

class CoreLifecycleSiblingsParentComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreLifecycleSiblingsParent";

    static inline std::vector<std::string>* events = nullptr;

    void onInit(drogular::RenderContext&) override {
        events->push_back("parent-init");
    }

    std::string templateHtml() const override {
        events->push_back("parent-render");

        return R"(<section><CoreLifecycleFirstChild /><CoreLifecycleSecondChild /></section>)";
    }

    void onDestroy(drogular::RenderContext&) override {
        events->push_back("parent-destroy");
    }
};

TEST(CoreComponentLifecycleTests, CallsLifecycleHooksInOrderForSiblingComponents) {
    std::vector<std::string> events;

    CoreLifecycleSiblingsParentComponent::events = &events;
    CoreLifecycleFirstChildComponent::events = &events;
    CoreLifecycleSecondChildComponent::events = &events;

    drogular::ApplicationServices services;

    services.components().registerComponent<CoreLifecycleSiblingsParentComponent>();
    services.components().registerComponent<CoreLifecycleFirstChildComponent>();
    services.components().registerComponent<CoreLifecycleSecondChildComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
        drogular::component_renderer::render(
            "<CoreLifecycleSiblingsParent />",
            services.components(),
            context
        );

    EXPECT_EQ(
        html,
        "<section><span>First</span><span>Second</span></section>"
    );

    const std::vector<std::string> expected = {
        "parent-init",
        "parent-render",
        "first-init",
        "first-render",
        "first-destroy",
        "second-init",
        "second-render",
        "second-destroy",
        "parent-destroy"
    };

    EXPECT_EQ(events, expected);
}