#include <drogular/template_runtime.hpp>
#include <drogular/render_context.hpp>

#include <gtest/gtest.h>
#include <json/json.h>

using namespace drogular::template_compiler;

TEST(CoreTemplateRuntimeTests, ResolvesEscapedVariable) {
    drogular::RenderContext context;

    context.set("title", std::string("<Hello>"));

    const auto value = resolveVariable("title", context);

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "&lt;Hello&gt;");
}

TEST(CoreTemplateRuntimeTests, ResolvesRawVariable) {
    drogular::RenderContext context;

    context.set("html", std::string("<strong>Hello</strong>"));

    const auto value = resolveRawVariable("html", context);

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "<strong>Hello</strong>");
}

TEST(CoreTemplateRuntimeTests, ResolvesJsonPath) {
    drogular::RenderContext context;

    Json::Value user;
    user["profile"]["name"] = "Vadim";

    context.set("user", user);

    const auto value = resolveVariable("user.profile.name", context);

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "Vadim");
}

TEST(CoreTemplateRuntimeTests, EvaluatesBoolCondition) {
    drogular::RenderContext context;

    context.set("show", true);

    EXPECT_TRUE(evaluateCondition("show", context));
}

TEST(CoreTemplateRuntimeTests, EvaluatesJsonCondition) {
    drogular::RenderContext context;

    Json::Value user;
    user["active"] = true;

    context.set("user", user);

    EXPECT_TRUE(evaluateCondition("user.active", context));
}

TEST(CoreTemplateRuntimeTests, ResolvesJsonValue) {
    drogular::RenderContext context;

    Json::Value user;
    user["profile"]["name"] = "Vadim";

    context.set("user", user);

    const auto value =
        resolveJsonValue("user.profile.name", context);

    ASSERT_TRUE(value.has_value());
    EXPECT_TRUE(value->isString());
    EXPECT_EQ(value->asString(), "Vadim");
}

TEST(CoreTemplateRuntimeTests, ResolvesVariableContainingQueryString) {
    drogular::RenderContext context;

    context.set(
        "url",
        std::string("/projects?search=port")
    );

    const auto value =
        resolveVariable(
            "url",
            context
        );

    ASSERT_TRUE(value.has_value());

    EXPECT_EQ(
        *value,
        "/projects?search=port"
    );
}

TEST(CoreTemplateRuntimeTests, EvaluatesComparisonOperators) {
    drogular::RenderContext context;
    context.set("page", 3);
    context.set("status", std::string("active"));

    EXPECT_TRUE(evaluateCondition("page > 1", context));
    EXPECT_TRUE(evaluateCondition("page >= 3", context));
    EXPECT_TRUE(evaluateCondition("page < 4", context));
    EXPECT_TRUE(evaluateCondition("page <= 3", context));
    EXPECT_TRUE(evaluateCondition("page == 3", context));
    EXPECT_TRUE(evaluateCondition("page != 2", context));

    EXPECT_TRUE(evaluateCondition(
        "status == \"active\"",
        context
    ));
    EXPECT_FALSE(evaluateCondition(
        "status == \"disabled\"",
        context
    ));
}

TEST(CoreTemplateRuntimeTests, EvaluatesLogicalOperatorsAndParentheses) {
    drogular::RenderContext context;
    context.set("page", 3);
    context.set("hasNext", true);
    context.set("disabled", false);

    EXPECT_TRUE(evaluateCondition(
        "page > 1 && hasNext",
        context
    ));
    EXPECT_TRUE(evaluateCondition(
        "disabled || page == 3",
        context
    ));
    EXPECT_TRUE(evaluateCondition(
        "!disabled",
        context
    ));
    EXPECT_TRUE(evaluateCondition(
        "(page > 1 && hasNext) || disabled",
        context
    ));
    EXPECT_FALSE(evaluateCondition(
        "!(page > 1)",
        context
    ));
}

TEST(CoreTemplateRuntimeTests, EvaluatesJsonFieldsAndLiterals) {
    drogular::RenderContext context;
    Json::Value user;
    user["role"] = "admin";
    user["active"] = true;
    user["score"] = 42;
    context.set("user", user);

    EXPECT_TRUE(evaluateCondition(
        "user.role == 'admin' && user.active",
        context
    ));
    EXPECT_TRUE(evaluateCondition(
        "user.score >= 40",
        context
    ));
    EXPECT_TRUE(evaluateCondition(
        "true",
        context
    ));
    EXPECT_FALSE(evaluateCondition(
        "false",
        context
    ));
    EXPECT_TRUE(evaluateCondition(
        "missing == null",
        context
    ));
}