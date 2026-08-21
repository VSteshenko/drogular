#include <drogular/compiled_template.hpp>
#include <drogular/render_context.hpp>

#include <gtest/gtest.h>

#include <json/json.h>

TEST(CoreCompiledTemplateTests, RendersText) {
    drogular::RenderContext context;

    const auto compiled =
        drogular::template_compiler::compile("Hello");

    EXPECT_EQ(
        compiled.render(context),
        "Hello"
    );
}

TEST(CoreCompiledTemplateTests, RendersVariable) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto compiled =
        drogular::template_compiler::compile("<h1>{{ title }}</h1>");

    EXPECT_EQ(
        compiled.render(context),
        "<h1>Hello</h1>"
    );
}

TEST(CoreCompiledTemplateTests, RendersRawVariable) {
    drogular::RenderContext context;

    context.set("content", std::string("<strong>Hello</strong>"));

    const auto compiled =
        drogular::template_compiler::compile("{{{ content }}}");

    EXPECT_EQ(
        compiled.render(context),
        "<strong>Hello</strong>"
    );
}

TEST(CoreCompiledTemplateTests, RendersIfTrueBranch) {
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

TEST(CoreCompiledTemplateTests, RendersIfFalseBranch) {
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

TEST(CoreCompiledTemplateTests, RendersForeach) {
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

TEST(CoreCompiledTemplateTests, RendersForeachJsonArrayValues) {
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

TEST(CoreCompiledTemplateTests, RendersForeachJsonObjects) {
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

namespace CoreCompiledTemplateTests_models {

struct Filter {
    std::string search;
    bool active = false;
};

Json::Value toJson(
    const Filter& filter
) {
    Json::Value json(Json::objectValue);

    json["search"] = filter.search;
    json["active"] = filter.active;

    return json;
}

} // namespace CoreCompiledTemplateTests_models

TEST(CoreCompiledTemplateTests, RendersFieldsFromCustomStructureStoredAsJson) {
    drogular::RenderContext context;

    context.setJson(
        "filters",
        CoreCompiledTemplateTests_models::Filter{
            .search = "portal",
            .active = true
        }
    );

    const auto compiled =
        drogular::template_compiler::compile(
            "{{ filters.search }} "
            "@if(filters.active)active@endif"
        );

    EXPECT_EQ(
        compiled.render(context),
        "portal active"
    );
}

TEST(CoreCompiledTemplateTests, RendersConditionalExpression) {
    drogular::RenderContext context;
    context.set("page", 2);
    context.set("hasNext", true);

    const auto compiled =
        drogular::template_compiler::compile(
            "@if((page > 1) && hasNext)<p>Next</p>@else<p>Done</p>@endif"
        );

    EXPECT_EQ(
        compiled.render(context),
        "<p>Next</p>"
    );
}

TEST(CoreCompiledTemplateTests, RendersForeachWithWhereCondition) {
    drogular::RenderContext context;

    Json::Value todos(Json::arrayValue);

    Json::Value first;
    first["title"] = "A";
    first["completed"] = false;
    todos.append(first);

    Json::Value second;
    second["title"] = "B";
    second["completed"] = true;
    todos.append(second);

    Json::Value third;
    third["title"] = "C";
    third["completed"] = false;
    todos.append(third);

    context.set("todos", todos);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(todo in todos where !todo.completed)"
        "{{ todo.title }}"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "AC");
}

TEST(CoreCompiledTemplateTests, ExposesForeachLoopMetadata) {
    drogular::RenderContext context;

    Json::Value items(Json::arrayValue);
    items.append("A");
    items.append("B");
    items.append("C");
    context.set("items", items);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(item in items)"
        "{{ loop.index }}:{{ loop.number }}:{{ loop.count }}:"
        "{{ loop.first }}:{{ loop.last }}={{ item }};"
        "@endforeach"
    );

    EXPECT_EQ(
        compiled.render(context),
        "0:1:3:true:false=A;"
        "1:2:3:false:false=B;"
        "2:3:3:false:true=C;"
    );
}

TEST(CoreCompiledTemplateTests, LoopMetadataUsesFilteredSequence) {
    drogular::RenderContext context;

    Json::Value items(Json::arrayValue);
    for (int value = 1; value <= 4; ++value) {
        Json::Value item;
        item["value"] = value;
        item["visible"] = value % 2 == 0;
        items.append(item);
    }
    context.set("items", items);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(item in items where item.visible)"
        "{{ loop.number }}/{{ loop.count }}={{ item.value }};"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "1/2=2;2/2=4;");
}