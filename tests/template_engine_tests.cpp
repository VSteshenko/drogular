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
