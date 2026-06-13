#include <drogular/compiled_template.hpp>

#include <gtest/gtest.h>

#include <json/json.h>

TEST(CompiledTemplateTests, RendersText) {
    drogular::RenderContext context;

    const auto compiled =
        drogular::template_compiler::compile("Hello");

    EXPECT_EQ(
        compiled.render(context),
        "Hello"
    );
}

TEST(CompiledTemplateTests, RendersVariable) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto compiled =
        drogular::template_compiler::compile("<h1>{{ title }}</h1>");

    EXPECT_EQ(
        compiled.render(context),
        "<h1>Hello</h1>"
    );
}

TEST(CompiledTemplateTests, RendersRawVariable) {
    drogular::RenderContext context;

    context.set("content", std::string("<strong>Hello</strong>"));

    const auto compiled =
        drogular::template_compiler::compile("{{{ content }}}");

    EXPECT_EQ(
        compiled.render(context),
        "<strong>Hello</strong>"
    );
}

TEST(CompiledTemplateTests, RendersIfTrueBranch) {
    drogular::RenderContext context;

    context.set("show", true);

    const auto compiled =
        drogular::template_compiler::compile(
            "@if(show)<p>Yes</p>@else<p>No</p>@endif"
        );

    EXPECT_EQ(
        compiled.render(context),
        "<p>Yes</p>"
    );
}

TEST(CompiledTemplateTests, RendersIfFalseBranch) {
    drogular::RenderContext context;

    context.set("show", false);

    const auto compiled =
        drogular::template_compiler::compile(
            "@if(show)<p>Yes</p>@else<p>No</p>@endif"
        );

    EXPECT_EQ(
        compiled.render(context),
        "<p>No</p>"
    );
}

TEST(CompiledTemplateTests, RendersForeach) {
    drogular::RenderContext context;

    context.set(
        "items",
        std::vector<std::string>{
            "A",
            "B"
        }
    );

    const auto compiled =
        drogular::template_compiler::compile(
            "@foreach(item in items)<p>{{ item }}</p>@endforeach"
        );

    EXPECT_EQ(
        compiled.render(context),
        "<p>A</p><p>B</p>"
    );
}

TEST(CompiledTemplateTests, RendersForeachJsonArrayValues) {
    drogular::RenderContext context;

    Json::Value items(Json::arrayValue);
    items.append("A");
    items.append("B");
    items.append("C");

    context.set("items", items);

    const auto compiled =
        drogular::template_compiler::compile(
            "@foreach(item in items)"
            "<p>{{ item }}</p>"
            "@endforeach"
        );

    EXPECT_EQ(
        compiled.render(context),
        "<p>A</p><p>B</p><p>C</p>"
    );
}

TEST(CompiledTemplateTests, RendersForeachJsonObjects) {
    drogular::RenderContext context;

    Json::Value todos(Json::arrayValue);

    Json::Value first;
    first["title"] = "Learn";

    Json::Value second;
    second["title"] = "Build";

    todos.append(first);
    todos.append(second);

    context.set("todos", todos);

    const auto compiled =
        drogular::template_compiler::compile(
            "@foreach(todo in todos)"
            "<p>{{ todo.title }}</p>"
            "@endforeach"
        );

    EXPECT_EQ(
        compiled.render(context),
        "<p>Learn</p><p>Build</p>"
    );
}