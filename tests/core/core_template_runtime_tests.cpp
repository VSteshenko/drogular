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

TEST(CoreTemplateRuntimeTests, ValidatesConditionExpressionSyntax) {
    EXPECT_FALSE(
        validateConditionExpression(
            "user.role == 'admin' && (page > 1 || hasNext)"
        ).has_value()
    );

    const auto missingValue = validateConditionExpression("page >");
    ASSERT_TRUE(missingValue.has_value());
    EXPECT_EQ(missingValue->message, "Expected value");

    const auto missingParenthesis =
        validateConditionExpression("(page > 1");
    ASSERT_TRUE(missingParenthesis.has_value());
    EXPECT_EQ(missingParenthesis->message, "Expected ')'");

    const auto unterminatedString =
        validateConditionExpression("status == 'active");
    ASSERT_TRUE(unterminatedString.has_value());
    EXPECT_EQ(
        unterminatedString->message,
        "Unterminated string literal"
    );
}

TEST(CoreTemplateRuntimeTests, ParsesForeachExpressionWithWhereCondition) {
    const auto expression = parseForeachExpression(
        "todo in todos where !todo.completed && todo.priority >= 2"
    );

    ASSERT_TRUE(expression.has_value());
    EXPECT_EQ(expression->variable, "todo");
    EXPECT_EQ(expression->collection, "todos");
    ASSERT_TRUE(expression->condition.has_value());
    EXPECT_EQ(
        *expression->condition,
        "!todo.completed && todo.priority >= 2"
    );
}

TEST(CoreTemplateRuntimeTests, ValidatesForeachExpressionSyntax) {
    EXPECT_FALSE(validateForeachExpression("item in items").has_value());
    EXPECT_FALSE(
        validateForeachExpression("item in items where item.active").has_value()
    );

    const auto missingIn = validateForeachExpression("item items");
    ASSERT_TRUE(missingIn.has_value());
    EXPECT_EQ(missingIn->message, "Expected 'in'");

    const auto missingCollection = validateForeachExpression("item in ");
    ASSERT_TRUE(missingCollection.has_value());
    EXPECT_EQ(missingCollection->message, "Expected collection after 'in'");

    const auto missingCondition =
        validateForeachExpression("item in items where ");
    ASSERT_TRUE(missingCondition.has_value());
    EXPECT_EQ(missingCondition->message, "Expected condition after 'where'");
}

TEST(CoreTemplateRuntimeTests, EvaluatesMembershipConditionsThroughRuntimeFacade) {
    drogular::RenderContext context;
    context.set("role", std::string("Admin"));
    context.set("page", 5);

    EXPECT_TRUE(evaluateCondition(
        "role in ['Admin', 'Moderator']",
        context
    ));
    EXPECT_TRUE(evaluateCondition("page in [1..10]", context));
    EXPECT_TRUE(evaluateCondition("page not in [6..<10]", context));
}