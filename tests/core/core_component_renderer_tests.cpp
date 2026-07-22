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

class CoreConditionalComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreConditional";

    std::string templateHtml() const override {
        return R"(@if(model.visible)
<section>
    @if(model.hasPrevious)<a href="{{ model.previousUrl }}">Previous</a>@endif
    <span>{{ model.title }}</span>
    @if(model.hasNext)<a href="{{ model.nextUrl }}">Next</a>@endif
</section>
@endif)";
    }
};

TEST(CoreComponentRendererTests, ProcessesNestedIfDirectivesInsideComponent) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CoreConditionalComponent>();

    drogular::RenderContext context;

    Json::Value model;
    model["visible"] = true;
    model["hasPrevious"] = true;
    model["previousUrl"] = "/items?page=1";
    model["title"] = "Page 2";
    model["hasNext"] = true;
    model["nextUrl"] = "/items?page=3";
    context.set("model", model);

    const auto html = drogular::component_renderer::render(
        "<CoreConditional />",
        registry,
        context
    );

    EXPECT_NE(html.find("Previous"), std::string::npos);
    EXPECT_NE(html.find("Page 2"), std::string::npos);
    EXPECT_NE(html.find("Next"), std::string::npos);
    EXPECT_EQ(html.find("@if"), std::string::npos);
    EXPECT_EQ(html.find("@endif"), std::string::npos);
}

class CoreLoopComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreLoop";

    std::string templateHtml() const override {
        return R"(<ul>
@foreach(item in items)
    @if(item.visible)<li>{{ item.label }}</li>@endif
@endforeach
</ul>)";
    }
};

TEST(CoreComponentRendererTests, ProcessesIfInsideForeachInsideComponent) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CoreLoopComponent>();

    drogular::RenderContext context;

    Json::Value items(Json::arrayValue);
    Json::Value first;
    first["label"] = "Visible";
    first["visible"] = true;
    items.append(first);

    Json::Value second;
    second["label"] = "Hidden";
    second["visible"] = false;
    items.append(second);

    context.set("items", items);

    const auto html = drogular::component_renderer::render(
        "<CoreLoop />",
        registry,
        context
    );

    EXPECT_NE(html.find("Visible"), std::string::npos);
    EXPECT_EQ(html.find("Hidden"), std::string::npos);
    EXPECT_EQ(html.find("@foreach"), std::string::npos);
    EXPECT_EQ(html.find("@if"), std::string::npos);
}

class CoreParentContextComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreParentContext";

    std::string templateHtml() const override {
        return "<p>{{ inheritedValue }}</p>";
    }
};

TEST(CoreComponentRendererTests, ComponentReadsParentRenderContext) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CoreParentContextComponent>();

    drogular::RenderContext context;
    context.set("inheritedValue", std::string("from-parent"));

    EXPECT_EQ(
        drogular::component_renderer::render(
            "<CoreParentContext />",
            registry,
            context
        ),
        "<p>from-parent</p>"
    );
}

class CoreOverrideContextComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreOverrideContext";

    std::string templateHtml() const override {
        return "<p>{{ title }}</p>";
    }
};

TEST(CoreComponentRendererTests, ComponentInputOverridesParentValue) {
    drogular::ComponentRegistry registry;
    registry.registerComponent<CoreOverrideContextComponent>();

    drogular::RenderContext context;
    context.set("title", std::string("Parent"));

    EXPECT_EQ(
        drogular::component_renderer::render(
            R"(<CoreOverrideContext title="Component" />)",
            registry,
            context
        ),
        "<p>Component</p>"
    );
}