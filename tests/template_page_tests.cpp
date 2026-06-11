#include <drogular/page.hpp>
#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <string>
#include <json/json.h>

class TestTemplatePage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        context.set("title", std::string("Hello TemplatePage"));
    }

    std::string templateHtml() const override {
        return "<h1>{{ title }}</h1>";
    }
};

TEST(TemplatePageTests, RendersTemplateWithContext) {
    TestTemplatePage page;
    drogular::RenderContext context;

    page.onInit(context);

    EXPECT_EQ(
        page.render(context),
        "<h1>Hello TemplatePage</h1>"
    );
}

class TemplatePageCardComponent final : public drogular::Component {
public:
    static constexpr auto tag = "Card";

    std::string render(drogular::RenderContext&) override {
        return "<article>Card from TemplatePage</article>";
    }
};

class ComponentTagTemplatePage final : public drogular::TemplatePage {
public:
    std::string templateHtml() const override {
        return "<main><Card /></main>";
    }
};

TEST(TemplatePageTests, RendersComponentTags) {
    drogular::ApplicationServices services;
    services.components().registerComponent<TemplatePageCardComponent>();

    ComponentTagTemplatePage page;
    drogular::RenderContext context;
    context.setServices(&services);

    EXPECT_EQ(
        page.render(context),
        "<main><article>Card from TemplatePage</article></main>"
    );
}

class TemplatePageCardWithInput final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CardWithInput";

    std::string templateHtml() const override {
        return "<article>{{ title }}</article>";
    }
};

class ComponentInputTemplatePage final : public drogular::TemplatePage {
public:
    std::string templateHtml() const override {
        return R"(<main><CardWithInput title="Welcome from Page" /></main>)";
    }
};

TEST(TemplatePageTests, RendersComponentTagsWithStringInputs) {
    drogular::ApplicationServices services;
    services.components().registerComponent<TemplatePageCardWithInput>();

    ComponentInputTemplatePage page;
    drogular::RenderContext context;
    context.setServices(&services);

    EXPECT_EQ(
        page.render(context),
        "<main><article>Welcome from Page</article></main>"
    );
}

class TodoItemComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "TodoItem";

    std::string templateHtml() const override {
        return "<li>{{ title }}</li>";
    }
};

class ComponentBindingForeachPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        Json::Value todos(Json::arrayValue);

        Json::Value first;
        first["title"] = "Learn Drogular";

        Json::Value second;
        second["title"] = "Build Components";

        todos.append(first);
        todos.append(second);

        context.set("todos", todos);
    }

    std::string templateHtml() const override {
        return R"(
<ul>
@foreach(todo in todos)
<TodoItem title="{{ todo.title }}" />
@endforeach
</ul>
)";
    }
};

TEST(TemplatePageTests, RendersComponentBindingsInsideForeach) {
    drogular::ApplicationServices services;
    services.components().registerComponent<TodoItemComponent>();

    ComponentBindingForeachPage page;

    drogular::RenderContext context;
    context.setServices(&services);

    page.onInit(context);

    const auto html = page.render(context);

    EXPECT_TRUE(
        drogular::test::contains(
            html,
            "<li>Learn Drogular</li>"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            html,
            "<li>Build Components</li>"
        )
    );
}

class SlotCardComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "SlotCard";

    std::string templateHtml() const override {
        return "<article><slot/></article>";
    }
};

class ComponentSlotTemplatePage final : public drogular::TemplatePage {
public:
    std::string templateHtml() const override {
        return R"(<main><SlotCard><p>Hello from page</p></SlotCard></main>)";
    }
};

TEST(TemplatePageTests, RendersComponentTagsWithDefaultSlot) {
    drogular::ApplicationServices services;
    services.components().registerComponent<SlotCardComponent>();

    ComponentSlotTemplatePage page;
    drogular::RenderContext context;
    context.setServices(&services);

    EXPECT_EQ(
        page.render(context),
        "<main><article><p>Hello from page</p></article></main>"
    );
}