#include <drogon/HttpRequest.h>
#include <drogular/component_renderer.hpp>
#include <drogular/component_registry.hpp>
#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

#include <gtest/gtest.h>

#include <json/json.h>

class CoreCardComponent final : public drogular::Component {
public:
    static constexpr auto tag = "CoreCard";

    std::string render(drogular::RenderContext&) override {
        return "<article>CoreCard</article>";
    }
};

TEST(CoreComponentRendererTests, RendersSelfClosingComponentTag) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CoreCardComponent>();

    drogular::RenderContext context;

    const auto html =
        drogular::component_renderer::render(
            "<div><CoreCard /></div>",
            registry,
            context
        );

    EXPECT_EQ(
        html,
        "<div><article>CoreCard</article></div>"
    );
}

TEST(CoreComponentRendererTests, LeavesUnknownComponentTagAsText) {
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

class CoreCardWithTitleComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreCardWithTitle";

    std::string templateHtml() const override {
        return "<article><h2>{{ title }}</h2><p>{{ subtitle }}</p></article>";
    }
};

TEST(CoreComponentRendererTests, PassesStringAttributesAsInputs) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CoreCardWithTitleComponent>();

    drogular::RenderContext context;

    const auto html =
        drogular::component_renderer::render(
            R"(<CoreCardWithTitle title="Welcome" subtitle="Hello" />)",
            registry,
            context
        );

    EXPECT_EQ(
        html,
        "<article><h2>Welcome</h2><p>Hello</p></article>"
    );
}

class CoreCardWithBoundTitleComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreBoundCard";

    std::string templateHtml() const override {
        return "<article>{{ title }}</article>";
    }
};

TEST(CoreComponentRendererTests, RendersAttributeBindings) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CoreCardWithBoundTitleComponent>();

    drogular::RenderContext context;

    Json::Value user;
    user["name"] = "Vadim";

    context.set("user", user);

    const auto html =
        drogular::component_renderer::render(
            R"(<CoreBoundCard title="{{ user.name }}" />)",
            registry,
            context
        );

    EXPECT_EQ(
        html,
        "<article>Vadim</article>"
    );
}

class CoreCardWithSlotComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreCardWithSlot";

    std::string templateHtml() const override {
        return "<article><h2>{{ title }}</h2><slot/></article>";
    }
};

TEST(CoreComponentRendererTests, RendersComponentWithDefaultSlot) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CoreCardWithSlotComponent>();

    drogular::RenderContext context;

    const auto html =
        drogular::component_renderer::render(
            R"(<CoreCardWithSlot title="Welcome"><p>Hello</p></CoreCardWithSlot>)",
            registry,
            context
        );

    EXPECT_EQ(
        html,
        "<article><h2>Welcome</h2><p>Hello</p></article>"
    );
}

class CoreNestedCardComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreNestedCard";

    std::string templateHtml() const override {
        return "<article><slot/></article>";
    }
};

class CoreNestedButtonComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreNestedButton";

    std::string templateHtml() const override {
        return "<button>{{ title }}</button>";
    }
};

TEST(CoreComponentRendererTests, RendersNestedComponentInsideSlot) {
    drogular::ComponentRegistry registry;

    registry.registerComponent<CoreNestedCardComponent>();
    registry.registerComponent<CoreNestedButtonComponent>();

    drogular::RenderContext context;

    const auto html =
        drogular::component_renderer::render(
            R"(<CoreNestedCard><CoreNestedButton title="Click" /></CoreNestedCard>)",
            registry,
            context
        );

    EXPECT_EQ(
        html,
        "<article><button>Click</button></article>"
    );
}

TEST(CoreComponentRenderContextTests, ReadsCookieFromRequest) {
    auto request =
        drogon::HttpRequest::newHttpRequest();

    request->addCookie("session_id", "abc123");

    drogular::RenderContext context;
    context.setRequest(request);

    const auto value =
        context.cookie("session_id");

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "abc123");
}