#include <drogular/render_context.hpp>
#include <drogular/template_engine.hpp>

#include <gtest/gtest.h>

#include <string>
#include <json/json.h>

TEST(CoreTemplateEngineTests, ReplacesStringVariable) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello Drogular"));

    const auto html =
        drogular::template_engine::render(
            "<h1>{{ title }}</h1>",
            context
        );

    EXPECT_EQ(html, "<h1>Hello Drogular</h1>");
}

TEST(CoreTemplateEngineTests, ReplacesMultipleVariables) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));
    context.set("subtitle", std::string("Template Engine"));

    const auto html =
        drogular::template_engine::render(
            "<h1>{{ title }}</h1><p>{{ subtitle }}</p>",
            context
        );

    EXPECT_EQ(
        html,
        "<h1>Hello</h1><p>Template Engine</p>"
    );
}

TEST(CoreTemplateEngineTests, SupportsVariablesWithoutSpaces) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto html =
        drogular::template_engine::render(
            "<h1>{{title}}</h1>",
            context
        );

    EXPECT_EQ(html, "<h1>Hello</h1>");
}

TEST(CoreTemplateEngineTests, LeavesBrokenExpressionAsText) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto html =
        drogular::template_engine::render(
            "<h1>{{ title</h1>",
            context
        );

    EXPECT_EQ(html, "<h1>{{ title</h1>");
}

TEST(CoreTemplateEngineTests, RemovesMissingVariables) {
    drogular::RenderContext context;

    const auto html =
        drogular::template_engine::render(
            "<h1>{{ title }}</h1>",
            context
        );

    EXPECT_EQ(html, "<h1></h1>");
}

TEST(CoreTemplateEngineTests, ReplacesIntegerVariable) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto html =
        drogular::template_engine::render(
            "<span>{{ count }}</span>",
            context
        );

    EXPECT_EQ(html, "<span>42</span>");
}

TEST(CoreTemplateEngineTests, ReplacesBooleanVariable) {
    drogular::RenderContext context;

    context.set("done", true);

    const auto html =
        drogular::template_engine::render(
            "<span>{{ done }}</span>",
            context
        );

    EXPECT_EQ(html, "<span>true</span>");
}

TEST(CoreTemplateEngineTests, ReplacesDoubleVariable) {
    drogular::RenderContext context;

    context.set("price", 3.14);

    const auto html =
        drogular::template_engine::render(
            "<span>{{ price }}</span>",
            context
        );

    EXPECT_EQ(html, "<span>3.140000</span>");
}

TEST(CoreTemplateEngineTests, EscapesHtmlInStringValues) {
    drogular::RenderContext context;

    context.set("title", std::string("<script>alert(1)</script>"));

    const auto html =
        drogular::template_engine::render(
            "<h1>{{ title }}</h1>",
            context
        );

    EXPECT_EQ(
        html,
        "<h1>&lt;script&gt;alert(1)&lt;/script&gt;</h1>"
    );
}

TEST(CoreTemplateEngineTests, EscapesHtmlSpecialCharacters) {
    drogular::RenderContext context;

    context.set("text", std::string("Tom & \"Jerry\" <Cat> 'Mouse'"));

    const auto html =
        drogular::template_engine::render(
            "<p>{{ text }}</p>",
            context
        );

    EXPECT_EQ(
        html,
        "<p>Tom &amp; &quot;Jerry&quot; &lt;Cat&gt; &#39;Mouse&#39;</p>"
    );
}

TEST(CoreTemplateEngineTests, RendersRawHtmlVariable) {
    drogular::RenderContext context;

    context.set("content", std::string("<strong>Hello</strong>"));

    const auto html =
        drogular::template_engine::render(
            "<div>{{{ content }}}</div>",
            context
        );

    EXPECT_EQ(
        html,
        "<div><strong>Hello</strong></div>"
    );
}

TEST(CoreTemplateEngineTests, EscapedAndRawVariablesBehaveDifferently) {
    drogular::RenderContext context;

    context.set("content", std::string("<strong>Hello</strong>"));

    const auto html =
        drogular::template_engine::render(
            "<p>{{ content }}</p><div>{{{ content }}}</div>",
            context
        );

    EXPECT_EQ(
        html,
        "<p>&lt;strong&gt;Hello&lt;/strong&gt;</p><div><strong>Hello</strong></div>"
    );
}

TEST(CoreTemplateEngineTests, LeavesBrokenRawExpressionAsText) {
    drogular::RenderContext context;

    context.set("content", std::string("<strong>Hello</strong>"));

    const auto html =
        drogular::template_engine::render(
            "<div>{{{ content }}</div>",
            context
        );

    EXPECT_EQ(
        html,
        "<div>{{{ content }}</div>"
    );
}

TEST(CoreTemplateEngineTests, RendersIfBlockWhenConditionIsTrue) {
    drogular::RenderContext context;

    context.set("showTitle", true);
    context.set("title", std::string("Hello"));

    const auto html =
        drogular::template_engine::render(
            "@if(showTitle)<h1>{{ title }}</h1>@endif",
            context
        );

    EXPECT_EQ(html, "<h1>Hello</h1>");
}

TEST(CoreTemplateEngineTests, SkipsIfBlockWhenConditionIsFalse) {
    drogular::RenderContext context;

    context.set("showTitle", false);
    context.set("title", std::string("Hello"));

    const auto html =
        drogular::template_engine::render(
            "@if(showTitle)<h1>{{ title }}</h1>@endif",
            context
        );

    EXPECT_EQ(html, "");
}

TEST(CoreTemplateEngineTests, SkipsIfBlockWhenConditionIsMissing) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto html =
        drogular::template_engine::render(
            "@if(showTitle)<h1>{{ title }}</h1>@endif",
            context
        );

    EXPECT_EQ(html, "");
}

TEST(CoreTemplateEngineTests, LeavesBrokenIfBlockAsText) {
    drogular::RenderContext context;

    context.set("showTitle", true);

    const auto html =
        drogular::template_engine::render(
            "@if(showTitle)<h1>Hello</h1>",
            context
        );

    EXPECT_EQ(html, "@if(showTitle)<h1>Hello</h1>");
}

TEST(CoreTemplateEngineTests, RendersIfElseTrueBlock) {
    drogular::RenderContext context;

    context.set("isLoggedIn", true);

    const auto html =
        drogular::template_engine::render(
            "@if(isLoggedIn)<p>Hello</p>@else<p>Please log in</p>@endif",
            context
        );

    EXPECT_EQ(html, "<p>Hello</p>");
}

TEST(CoreTemplateEngineTests, RendersIfElseFalseBlock) {
    drogular::RenderContext context;

    context.set("isLoggedIn", false);

    const auto html =
        drogular::template_engine::render(
            "@if(isLoggedIn)<p>Hello</p>@else<p>Please log in</p>@endif",
            context
        );

    EXPECT_EQ(html, "<p>Please log in</p>");
}

TEST(CoreTemplateEngineTests, RendersElseBlockWhenConditionIsMissing) {
    drogular::RenderContext context;

    const auto html =
        drogular::template_engine::render(
            "@if(isLoggedIn)<p>Hello</p>@else<p>Please log in</p>@endif",
            context
        );

    EXPECT_EQ(html, "<p>Please log in</p>");
}

TEST(CoreTemplateEngineTests, RendersInlineIfInsideHtmlAttribute) {
    drogular::RenderContext context;
    context.set("selected", true);

    EXPECT_EQ(
        drogular::template_engine::render("<option @if(selected)selected@endif>Admin</option>",
            context
        ),
        "<option selected>Admin</option>"
    );
}

TEST(CoreTemplateEngineTests, RendersForeachBlock) {
    drogular::RenderContext context;

    context.set("items",
        std::vector<std::string>{
            "A",
            "B",
            "C"
        }
    );

    const auto html =
        drogular::template_engine::render(
            R"(
<ul>
@foreach(item in items)
<li>{{ item }}</li>
@endforeach
</ul>
)",
            context
        );

    EXPECT_TRUE(html.find("<li>A</li>") != std::string::npos);
    EXPECT_TRUE(html.find("<li>B</li>") != std::string::npos);
    EXPECT_TRUE(html.find("<li>C</li>") != std::string::npos);
}

TEST(CoreTemplateEngineTests, EscapesForeachValues) {
    drogular::RenderContext context;

    context.set("items", std::vector<std::string>{"<script>"});

    const auto html =
        drogular::template_engine::render(
            "@foreach(item in items)"
            "{{ item }}"
            "@endforeach",
            context
        );

    EXPECT_EQ(html, "&lt;script&gt;");
}

TEST(CoreTemplateEngineTests, RendersJsonObjectProperty) {
    drogular::RenderContext context;

    Json::Value user;
    user["name"] = "Vadim";

    context.set("user", user);

    const auto html =
        drogular::template_engine::render(
            "<h1>{{ user.name }}</h1>",
            context
        );

    EXPECT_EQ(html, "<h1>Vadim</h1>");
}

TEST(CoreTemplateEngineTests, RendersNestedJsonObjectProperty) {
    drogular::RenderContext context;

    Json::Value user;
    user["profile"]["email"] = "vadim@example.com";

    context.set("user", user);

    const auto html =
        drogular::template_engine::render(
            "<p>{{ user.profile.email }}</p>",
            context
        );

    EXPECT_EQ(html, "<p>vadim@example.com</p>");
}

TEST(CoreTemplateEngineTests, EscapesJsonStringProperty) {
    drogular::RenderContext context;

    Json::Value user;
    user["name"] = "<Vadim>";

    context.set("user", user);

    const auto html =
        drogular::template_engine::render(
            "<h1>{{ user.name }}</h1>",
            context
        );

    EXPECT_EQ(html, "<h1>&lt;Vadim&gt;</h1>");
}

TEST(CoreTemplateEngineTests, RendersJsonNumberAndBoolProperties) {
    drogular::RenderContext context;

    Json::Value user;
    user["age"] = 49;
    user["active"] = true;

    context.set("user", user);

    const auto html =
        drogular::template_engine::render(
            "{{ user.age }} {{ user.active }}",
            context
        );

    EXPECT_EQ(html, "49 true");
}

TEST(CoreTemplateEngineTests, RemovesMissingJsonProperty) {
    drogular::RenderContext context;

    Json::Value user;
    user["name"] = "Vadim";

    context.set("user", user);

    const auto html =
        drogular::template_engine::render(
            "<p>{{ user.email }}</p>",
            context
        );

    EXPECT_EQ(html, "<p></p>");
}

TEST(CoreTemplateEngineTests, RendersForeachJsonArrayObjects) {
    drogular::RenderContext context;

    Json::Value todos(Json::arrayValue);

    Json::Value first;
    first["title"] = "Learn Drogular";

    Json::Value second;
    second["title"] = "Build PWA";

    todos.append(first);
    todos.append(second);

    context.set("todos", todos);

    const auto html =
        drogular::template_engine::render(
            "@foreach(todo in todos)"
            "<li>{{ todo.title }}</li>"
            "@endforeach",
            context
        );

    EXPECT_EQ(html, "<li>Learn Drogular</li><li>Build PWA</li>");
}

TEST(CoreTemplateEngineTests, RendersForeachJsonArrayScalarValues) {
    drogular::RenderContext context;

    Json::Value items(Json::arrayValue);
    items.append("A");
    items.append("B");

    context.set("items", items);

    const auto html =
        drogular::template_engine::render(
            "@foreach(item in items)"
            "<span>{{ item }}</span>"
            "@endforeach",
            context
        );

    EXPECT_EQ(html, "<span>A</span><span>B</span>");
}

TEST(CoreTemplateEngineTests, EscapesForeachJsonObjectValues) {
    drogular::RenderContext context;

    Json::Value todos(Json::arrayValue);

    Json::Value todo;
    todo["title"] = "<script>";

    todos.append(todo);

    context.set("todos", todos);

    const auto html =
        drogular::template_engine::render(
            "@foreach(todo in todos)"
            "{{ todo.title }}"
            "@endforeach",
            context
        );

    EXPECT_EQ(html, "&lt;script&gt;");
}

TEST(CoreTemplateEngineTests, RendersIfBlockForJsonBoolProperty) {
    drogular::RenderContext context;

    Json::Value user;
    user["active"] = true;

    context.set("user", user);

    const auto html =
        drogular::template_engine::render(
            "@if(user.active)<p>Active</p>@endif",
            context
        );

    EXPECT_EQ(html, "<p>Active</p>");
}

TEST(CoreTemplateEngineTests, SkipsIfBlockForJsonFalseProperty) {
    drogular::RenderContext context;

    Json::Value user;
    user["active"] = false;

    context.set("user", user);

    const auto html =
        drogular::template_engine::render(
            "@if(user.active)<p>Active</p>@endif",
            context
        );

    EXPECT_EQ(html, "");
}

TEST(CoreTemplateEngineTests, RendersElseBlockForJsonFalseProperty) {
    drogular::RenderContext context;

    Json::Value user;
    user["active"] = false;

    context.set("user", user);

    const auto html =
        drogular::template_engine::render(
            "@if(user.active)<p>Active</p>@else<p>Inactive</p>@endif",
            context
        );

    EXPECT_EQ(html, "<p>Inactive</p>");
}

TEST(CoreTemplateEngineTests, RendersIfInsideJsonForeach) {
    drogular::RenderContext context;

    Json::Value todos(Json::arrayValue);

    Json::Value first;
    first["title"] = "Learn Drogular";
    first["done"] = true;

    Json::Value second;
    second["title"] = "Build PWA";
    second["done"] = false;

    todos.append(first);
    todos.append(second);

    context.set("todos", todos);

    const auto html =
        drogular::template_engine::render(
            "@foreach(todo in todos)"
            "@if(todo.done)"
            "<li>[x] {{ todo.title }}</li>"
            "@else"
            "<li>[ ] {{ todo.title }}</li>"
            "@endif"
            "@endforeach",
            context
        );

    EXPECT_EQ(html, "<li>[x] Learn Drogular</li><li>[ ] Build PWA</li>");
}

TEST(CoreTemplateEngineTests, RendersDeepJsonObjectProperty) {
    drogular::RenderContext context;

    Json::Value user;
    user["profile"]["contact"]["email"] = "vadim@example.com";

    context.set("user", user);

    const auto html =
        drogular::template_engine::render(
            "{{ user.profile.contact.email }}",
            context
        );

    EXPECT_EQ(html, "vadim@example.com");
}

TEST(CoreTemplateEngineTests, RendersDeepJsonPropertyInsideForeach) {
    drogular::RenderContext context;

    Json::Value todos(Json::arrayValue);

    Json::Value todo;
    todo["title"] = "Learn Drogular";
    todo["author"]["profile"]["name"] = "Vadim";

    todos.append(todo);

    context.set("todos", todos);

    const auto html =
        drogular::template_engine::render(
            "@foreach(todo in todos)"
            "{{ todo.author.profile.name }}: {{ todo.title }}"
            "@endforeach",
            context
        );

    EXPECT_EQ(html, "Vadim: Learn Drogular");
}

TEST(CoreTemplateEngineTests, RendersDeepJsonConditionInsideForeach) {
    drogular::RenderContext context;

    Json::Value todos(Json::arrayValue);

    Json::Value todo;
    todo["title"] = "Learn Drogular";
    todo["meta"]["done"] = true;

    todos.append(todo);

    context.set("todos", todos);

    const auto html =
        drogular::template_engine::render(
            "@foreach(todo in todos)"
            "@if(todo.meta.done)"
            "{{ todo.title }}"
            "@endif"
            "@endforeach",
            context
        );

    EXPECT_EQ(html, "Learn Drogular");
}

TEST(CoreTemplateEngineTests, RendersConditionalExpression) {
    drogular::RenderContext context;
    context.set("count", 4);
    context.set("enabled", true);

    const auto html = drogular::template_engine::render(
        "@if((count >= 4) && enabled)yes@elseno@endif",
        context
    );

    EXPECT_EQ(html, "yes");
}

TEST(CoreTemplateEngineTests, RendersForeachWithWhereCondition) {
    drogular::RenderContext context;

    Json::Value items(Json::arrayValue);
    for (int value = 1; value <= 3; ++value) {
        Json::Value item;
        item["value"] = value;
        item["visible"] = value != 2;
        items.append(item);
    }
    context.set("items", items);

    const auto html = drogular::template_engine::render(
        "@foreach(item in items where item.visible)"
        "{{ item.value }}"
        "@endforeach",
        context
    );

    EXPECT_EQ(html, "13");
}

TEST(CoreTemplateEngineTests, ExposesForeachLoopMetadata) {
    drogular::RenderContext context;

    Json::Value items(Json::arrayValue);
    items.append("A");
    items.append("B");
    context.set("items", items);

    const auto html = drogular::template_engine::render(
        "@foreach(item in items)"
        "{{ loop.number }}/{{ loop.count }}:{{ loop.first }}:{{ loop.last }}={{ item }};"
        "@endforeach",
        context
    );

    EXPECT_EQ(html, "1/2:true:false=A;2/2:false:true=B;");
}

TEST(CoreTemplateEngineTests, ExposesParentAndDepthForNestedForeach) {
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

    const auto html = drogular::template_engine::render(
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
        "];@endforeach",
        context
    );

    EXPECT_EQ(
        html,
        "0:1:root[1:1.1[2:1.1.1=A;]];"
        "0:2:root[1:2.1[2:2.1.1=B;]];"
    );
}

TEST(CoreTemplateEngineTests, RootLoopIgnoresUserLoopValueAsParent) {
    drogular::RenderContext context;

    Json::Value userLoop(Json::objectValue);
    userLoop["depth"] = 99;
    context.set("loop", userLoop);

    Json::Value items(Json::arrayValue);
    items.append("A");
    context.set("items", items);

    const auto html = drogular::template_engine::render(
        "@foreach(item in items)"
        "{{ loop.depth }}:@if(loop.parent)parent@elseroot@endif"
        "@endforeach",
        context
    );

    EXPECT_EQ(html, "0:root");
}

TEST(CoreTemplateEngineTests, RendersForeachEmptyBranch) {
    drogular::RenderContext context;
    Json::Value items(Json::arrayValue);
    context.set("items", items);

    const auto html = drogular::template_engine::render(
        "@foreach(item in items){{ item }}@empty<p>Empty</p>@endforeach",
        context
    );

    EXPECT_EQ(html, "<p>Empty</p>");
}

TEST(CoreTemplateEngineTests, SupportsContinueInsideForeach) {
    drogular::RenderContext context;
    Json::Value items(Json::arrayValue);
    for (int value = 1; value <= 4; ++value) {
        Json::Value item;
        item["value"] = value;
        item["skip"] = value % 2 == 0;
        items.append(item);
    }
    context.set("items", items);

    const auto html = drogular::template_engine::render(
        "@foreach(item in items)"
        "@if(item.skip)@continue@endif"
        "{{ item.value }}"
        "@endforeach",
        context
    );

    EXPECT_EQ(html, "13");
}

TEST(CoreTemplateEngineTests, ContinueOnlySkipsNearestForeachIteration) {
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

    const auto html = drogular::template_engine::render(
        "@foreach(row in rows)"
        "{{ row.value }}["
        "@foreach(cell in row.cells)"
        "@if(cell == \"B\")@continue@endif"
        "{{ cell }}"
        "@endforeach"
        "];"
        "@endforeach",
        context
    );

    EXPECT_EQ(html, "1[AC];2[AC];");
}

TEST(CoreTemplateEngineTests, SupportsBreakInsideForeach) {
    drogular::RenderContext context;
    Json::Value items(Json::arrayValue);
    items.append(1);
    items.append(2);
    items.append(3);
    items.append(4);
    context.set("items", items);

    const auto html = drogular::template_engine::render(
        "@foreach(item in items)"
        "@if(item == 3)@break@endif"
        "{{ item }}"
        "@endforeach",
        context
    );

    EXPECT_EQ(html, "12");
}