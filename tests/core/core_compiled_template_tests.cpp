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

TEST(CoreCompiledTemplateTests, ExposesParentAndDepthForNestedForeach) {
    drogular::RenderContext context;

    Json::Value rows(Json::arrayValue);
    for (int rowIndex = 0; rowIndex < 2; ++rowIndex) {
        Json::Value row;
        Json::Value cells(Json::arrayValue);
        Json::Value cell;
        Json::Value tags(Json::arrayValue);
        tags.append(rowIndex == 0 ? "A" : "B");
        cell["tags"] = tags;
        cells.append(cell);
        row["cells"] = cells;
        rows.append(row);
    }
    context.set("rows", rows);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(row in rows)"
        "{{ loop.depth }}:{{ loop.number }}:"
        "@if(loop.parent)parent@elseroot@endif["
        "@foreach(cell in row.cells)"
        "{{ loop.depth }}:{{ loop.parent.number }}.{{ loop.number }}["
        "@foreach(tag in cell.tags)"
        "{{ loop.depth }}:{{ loop.parent.parent.number }}."
        "{{ loop.parent.number }}.{{ loop.number }}={{ tag }};"
        "@endforeach"
        "]@endforeach"
        "];@endforeach"
    );

    EXPECT_EQ(
        compiled.render(context),
        "0:1:root[1:1.1[2:1.1.1=A;]];"
        "0:2:root[1:2.1[2:2.1.1=B;]];"
    );
}

TEST(CoreCompiledTemplateTests, RootLoopIgnoresUserLoopValueAsParent) {
    drogular::RenderContext context;

    Json::Value userLoop(Json::objectValue);
    userLoop["depth"] = 99;
    context.set("loop", userLoop);

    Json::Value items(Json::arrayValue);
    items.append("A");
    context.set("items", items);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(item in items)"
        "{{ loop.depth }}:@if(loop.parent)parent@elseroot@endif"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "0:root");
}

TEST(CoreCompiledTemplateTests, RendersForeachEmptyBranch) {
    drogular::RenderContext context;
    Json::Value items(Json::arrayValue);
    context.set("items", items);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(item in items){{ item }}@empty<p>Empty</p>@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "<p>Empty</p>");
}

TEST(CoreCompiledTemplateTests, ForeachEmptyUsesFilteredSequence) {
    drogular::RenderContext context;
    Json::Value items(Json::arrayValue);
    Json::Value item;
    item["visible"] = false;
    items.append(item);
    context.set("items", items);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(item in items where item.visible)visible@emptyempty@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "empty");
}

TEST(CoreCompiledTemplateTests, SupportsContinueInsideForeach) {
    drogular::RenderContext context;
    Json::Value items(Json::arrayValue);
    for (int value = 1; value <= 4; ++value) {
        Json::Value item;
        item["value"] = value;
        item["skip"] = value % 2 == 0;
        items.append(item);
    }
    context.set("items", items);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(item in items)"
        "@if(item.skip)@continue@endif"
        "{{ item.value }}"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "13");
}

TEST(CoreCompiledTemplateTests, SupportsBreakInsideForeach) {
    drogular::RenderContext context;
    Json::Value items(Json::arrayValue);
    for (int value = 1; value <= 4; ++value) {
        Json::Value item;
        item["value"] = value;
        items.append(item);
    }
    context.set("items", items);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(item in items)"
        "@if(item.value == 3)@break@endif"
        "{{ item.value }}"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "12");
}

TEST(CoreCompiledTemplateTests, ContinueOnlySkipsNearestForeachIteration) {
    drogular::RenderContext context;

    Json::Value rows(Json::arrayValue);
    for (int rowValue = 1; rowValue <= 2; ++rowValue) {
        Json::Value row;
        row["value"] = rowValue;
        Json::Value cells(Json::arrayValue);
        cells.append("A");
        cells.append("B");
        cells.append("C");
        row["cells"] = cells;
        rows.append(row);
    }
    context.set("rows", rows);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(row in rows)"
        "{{ row.value }}["
        "@foreach(cell in row.cells)"
        "@if(cell == \"B\")@continue@endif"
        "{{ cell }}"
        "@endforeach"
        "];"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "1[AC];2[AC];");
}

TEST(CoreCompiledTemplateTests, BreakOnlyStopsNearestForeach) {
    drogular::RenderContext context;

    Json::Value rows(Json::arrayValue);
    for (int rowValue = 1; rowValue <= 2; ++rowValue) {
        Json::Value row;
        row["value"] = rowValue;
        Json::Value cells(Json::arrayValue);
        cells.append("A");
        cells.append("B");
        cells.append("C");
        row["cells"] = cells;
        rows.append(row);
    }
    context.set("rows", rows);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(row in rows)"
        "{{ row.value }}["
        "@foreach(cell in row.cells)"
        "@if(cell == \"B\")@break@endif"
        "{{ cell }}"
        "@endforeach"
        "];"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "1[A];2[A];");
}

TEST(CoreCompiledTemplateTests, ForeachAcceptsListExpression) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(value in [1, 3, 5, 7]){{ value }}@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "1357");
}

TEST(CoreCompiledTemplateTests, ForeachAcceptsComputedRangeExpression) {
    drogular::RenderContext context;
    context.set("start", 2);
    context.set("count", 4);

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(value in [start..start + count - 1])"
        "{{ loop.number }}={{ value }};"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "1=2;2=3;3=4;4=5;");
}

TEST(CoreCompiledTemplateTests, ForeachWhereFiltersExpressionIterable) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(value in [1..10] where value in [2, 4, 6, 8])"
        "{{ value }}"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "2468");
}

TEST(CoreCompiledTemplateTests, ForeachEmptySupportsEmptyExpressionList) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(value in []){{ value }}@emptyempty@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "empty");
}

TEST(CoreCompiledTemplateTests, ForeachPreservesNestedExpressionIterables) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(row in [[1, 2], [3, 4]])"
        "@foreach(cell in row){{ cell }}@endforeach;"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "12;34;");
}

TEST(CoreCompiledTemplateTests, RendersExpressionMethodCallInInterpolation) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "{{ [1, 2, 3].count() }}:{{ [1..5].last() }}"
    );

    EXPECT_EQ(compiled.render(context), "3:5");
}

TEST(CoreCompiledTemplateTests, RendersLetBinding) {
    drogular::RenderContext context;
    context.set("count", 5);

    const auto compiled = drogular::template_compiler::compile(
        "@let(pages = [0..<count]){{ pages.count() }}:{{ pages.last() }}"
    );

    EXPECT_EQ(compiled.render(context), "5:4");
}

TEST(CoreCompiledTemplateTests, LetBindingPersistsToEndOfCurrentBlock) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "@let(value = 10){{ value }}-{{ value + 1 }}"
    );

    EXPECT_EQ(compiled.render(context), "10-11");
}

TEST(CoreCompiledTemplateTests, LetBindingShadowsOnlyInsideIfScope) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "@let(value = 1)"
        "{{ value }}"
        "@if(true)@let(value = 2){{ value }}@endif"
        "{{ value }}"
    );

    EXPECT_EQ(compiled.render(context), "121");
}

TEST(CoreCompiledTemplateTests, LetBindingIsScopedToForeachIteration) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "@foreach(i in [1..3])"
        "@let(number = i * 10){{ number }};"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "10;20;30;");
}

TEST(CoreCompiledTemplateTests, LetCanUseLoopAndOuterBindings) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "@let(multiplier = 10)"
        "@foreach(i in [1..2])"
        "@let(label = loop.number * multiplier + i)"
        "{{ label }};"
        "@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "11;22;");
}

TEST(CoreCompiledTemplateTests, LetDoesNotMutateRenderContext) {
    drogular::RenderContext context;
    context.set("value", 7);

    const auto compiled = drogular::template_compiler::compile(
        "@if(true)@let(value = 9){{ value }}@endif{{ value }}"
    );

    EXPECT_EQ(compiled.render(context), "97");
    ASSERT_TRUE(context.get<int>("value").has_value());
    EXPECT_EQ(*context.get<int>("value"), 7);
}

TEST(CoreCompiledTemplateTests, RendersConstBinding) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "@const(PageSize = 3)"
        "{{ PageSize }}:"
        "@foreach(i in [1..PageSize]){{ i }}@endforeach"
    );

    EXPECT_EQ(compiled.render(context), "3:123");
}

TEST(CoreCompiledTemplateTests, ConstCanUseExpressionFunctions) {
    drogular::RenderContext context;
    context.set("count", 5);

    const auto compiled = drogular::template_compiler::compile(
        "@const(pages = [0..<count])"
        "{{ pages.count() }}:{{ pages.last() }}"
    );

    EXPECT_EQ(compiled.render(context), "5:4");
}

TEST(CoreCompiledTemplateTests, ConstCanBeShadowedInNestedScope) {
    drogular::RenderContext context;

    const auto compiled = drogular::template_compiler::compile(
        "@const(value = 1)"
        "{{ value }}"
        "@if(true)@const(value = 2){{ value }}@endif"
        "{{ value }}"
    );

    EXPECT_EQ(compiled.render(context), "121");
}