#include <drogular/template_expression.hpp>
#include <drogular/render_context.hpp>

#include <gtest/gtest.h>
#include <json/json.h>

using namespace drogular::template_expression;

TEST(CoreTemplateExpressionTests, ParsesExpressionIntoAst) {
    const auto result = parse("user.active && page >= 2");

    ASSERT_TRUE(result);
    ASSERT_NE(result.expression, nullptr);

    const auto* root =
        std::get_if<BinaryExpression>(&result.expression->node);
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->op, BinaryOperator::And);

    const auto* left =
        std::get_if<VariableExpression>(&root->left->node);
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->path, "user.active");

    const auto* right =
        std::get_if<BinaryExpression>(&root->right->node);
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->op, BinaryOperator::GreaterEqual);
}

TEST(CoreTemplateExpressionTests, EvaluatesParsedAst) {
    drogular::RenderContext context;
    context.set("page", 3);

    Json::Value user;
    user["active"] = true;
    context.set("user", user);

    const auto parsed = parse("user.active && page >= 2");
    ASSERT_TRUE(parsed);

    EXPECT_TRUE(evaluate(*parsed.expression, context).truthy());
}

TEST(CoreTemplateExpressionTests, ResolvesDottedJsonPath) {
    drogular::RenderContext context;

    Json::Value user;
    user["profile"]["score"] = 42;
    context.set("user", user);

    const auto value = resolve("user.profile.score", context);

    ASSERT_TRUE(value.number().has_value());
    EXPECT_DOUBLE_EQ(*value.number(), 42.0);
}

TEST(CoreTemplateExpressionTests, PreservesExpressionDiagnostics) {
    const auto missingValue = parse("page >");
    ASSERT_FALSE(missingValue);
    ASSERT_TRUE(missingValue.error.has_value());
    EXPECT_EQ(missingValue.error->message, "Expected value after '>'");
    EXPECT_EQ(missingValue.error->position, 6u);

    const auto missingParenthesis = parse("(page > 1");
    ASSERT_FALSE(missingParenthesis);
    ASSERT_TRUE(missingParenthesis.error.has_value());
    EXPECT_EQ(missingParenthesis.error->message, "Expected ')'");

    const auto unterminatedString = parse("status == 'active");
    ASSERT_FALSE(unterminatedString);
    ASSERT_TRUE(unterminatedString.error.has_value());
    EXPECT_EQ(
        unterminatedString.error->message,
        "Unterminated string literal"
    );
}

TEST(CoreTemplateExpressionTests, MissingValueBehavesAsNull) {
    drogular::RenderContext context;

    const auto parsed = parse("missing == null");
    ASSERT_TRUE(parsed);

    EXPECT_TRUE(evaluate(*parsed.expression, context).truthy());
}

TEST(CoreTemplateExpressionTests, EvaluatesLogicalPrecedence) {
    drogular::RenderContext context;
    context.set("a", false);
    context.set("b", true);
    context.set("c", false);

    const auto parsed = parse("a || b && !c");
    ASSERT_TRUE(parsed);

    EXPECT_TRUE(evaluate(*parsed.expression, context).truthy());
}