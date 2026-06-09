#include <drogular/component.hpp>
#include <drogular/template_engine.hpp>

#include <gtest/gtest.h>

#include <string>

TEST(TemplateEngineTests, ReplacesStringVariable) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello Drogular"));

    const auto html =
        drogular::template_engine::render(
            "<h1>{{ title }}</h1>",
            context
        );

    EXPECT_EQ(html, "<h1>Hello Drogular</h1>");
}

TEST(TemplateEngineTests, ReplacesMultipleVariables) {
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

TEST(TemplateEngineTests, SupportsVariablesWithoutSpaces) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto html =
        drogular::template_engine::render(
            "<h1>{{title}}</h1>",
            context
        );

    EXPECT_EQ(html, "<h1>Hello</h1>");
}

TEST(TemplateEngineTests, LeavesBrokenExpressionAsText) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto html =
        drogular::template_engine::render(
            "<h1>{{ title</h1>",
            context
        );

    EXPECT_EQ(html, "<h1>{{ title</h1>");
}

TEST(TemplateEngineTests, RemovesMissingVariables) {
    drogular::RenderContext context;

    const auto html =
        drogular::template_engine::render(
            "<h1>{{ title }}</h1>",
            context
        );

    EXPECT_EQ(html, "<h1></h1>");
}

TEST(TemplateEngineTests, ReplacesIntegerVariable) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto html =
        drogular::template_engine::render(
            "<span>{{ count }}</span>",
            context
        );

    EXPECT_EQ(html, "<span>42</span>");
}

TEST(TemplateEngineTests, ReplacesBooleanVariable) {
    drogular::RenderContext context;

    context.set("done", true);

    const auto html =
        drogular::template_engine::render(
            "<span>{{ done }}</span>",
            context
        );

    EXPECT_EQ(html, "<span>true</span>");
}

TEST(TemplateEngineTests, ReplacesDoubleVariable) {
    drogular::RenderContext context;

    context.set("price", 3.14);

    const auto html =
        drogular::template_engine::render(
            "<span>{{ price }}</span>",
            context
        );

    EXPECT_EQ(html, "<span>3.140000</span>");
}

TEST(TemplateEngineTests, EscapesHtmlInStringValues) {
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

TEST(TemplateEngineTests, EscapesHtmlSpecialCharacters) {
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

TEST(TemplateEngineTests, RendersRawHtmlVariable) {
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

TEST(TemplateEngineTests, EscapedAndRawVariablesBehaveDifferently) {
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

TEST(TemplateEngineTests, LeavesBrokenRawExpressionAsText) {
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

TEST(TemplateEngineTests, RendersIfBlockWhenConditionIsTrue) {
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

TEST(TemplateEngineTests, SkipsIfBlockWhenConditionIsFalse) {
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

TEST(TemplateEngineTests, SkipsIfBlockWhenConditionIsMissing) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto html =
        drogular::template_engine::render(
            "@if(showTitle)<h1>{{ title }}</h1>@endif",
            context
        );

    EXPECT_EQ(html, "");
}

TEST(TemplateEngineTests, LeavesBrokenIfBlockAsText) {
    drogular::RenderContext context;

    context.set("showTitle", true);

    const auto html =
        drogular::template_engine::render(
            "@if(showTitle)<h1>Hello</h1>",
            context
        );

    EXPECT_EQ(html, "@if(showTitle)<h1>Hello</h1>");
}

TEST(TemplateEngineTests, RendersIfElseTrueBlock) {
    drogular::RenderContext context;

    context.set("isLoggedIn", true);

    const auto html =
        drogular::template_engine::render(
            "@if(isLoggedIn)<p>Hello</p>@else<p>Please log in</p>@endif",
            context
        );

    EXPECT_EQ(html, "<p>Hello</p>");
}

TEST(TemplateEngineTests, RendersIfElseFalseBlock) {
    drogular::RenderContext context;

    context.set("isLoggedIn", false);

    const auto html =
        drogular::template_engine::render(
            "@if(isLoggedIn)<p>Hello</p>@else<p>Please log in</p>@endif",
            context
        );

    EXPECT_EQ(html, "<p>Please log in</p>");
}

TEST(TemplateEngineTests, RendersElseBlockWhenConditionIsMissing) {
    drogular::RenderContext context;

    const auto html =
        drogular::template_engine::render(
            "@if(isLoggedIn)<p>Hello</p>@else<p>Please log in</p>@endif",
            context
        );

    EXPECT_EQ(html, "<p>Please log in</p>");
}

TEST(TemplateEngineTests, RendersForeachBlock) {
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

TEST(TemplateEngineTests, EscapesForeachValues) {
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