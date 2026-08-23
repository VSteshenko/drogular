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
    EXPECT_EQ(missingValue.error->message, "Expected value");
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

TEST(CoreTemplateExpressionTests, EvaluatesArithmeticWithPrecedence) {
    drogular::RenderContext context;
    context.set("page", 2);
    context.set("size", 10);

    const auto value = evaluate("page * size + 3", context);

    ASSERT_TRUE(value.number().has_value());
    EXPECT_DOUBLE_EQ(*value.number(), 23.0);

    const auto grouped = evaluate("(page + 1) * size", context);
    ASSERT_TRUE(grouped.number().has_value());
    EXPECT_DOUBLE_EQ(*grouped.number(), 30.0);
}

TEST(CoreTemplateExpressionTests, EvaluatesListLiteralExpressions) {
    drogular::RenderContext context;
    context.set("page", 2);

    const auto value = evaluate("[1, page, page + 1, -4]", context);
    const auto array = value.array();

    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array->values.size(), 4u);
    ASSERT_TRUE(array->values[0].number().has_value());
    ASSERT_TRUE(array->values[1].number().has_value());
    ASSERT_TRUE(array->values[2].number().has_value());
    ASSERT_TRUE(array->values[3].number().has_value());
    EXPECT_DOUBLE_EQ(*array->values[0].number(), 1.0);
    EXPECT_DOUBLE_EQ(*array->values[1].number(), 2.0);
    EXPECT_DOUBLE_EQ(*array->values[2].number(), 3.0);
    EXPECT_DOUBLE_EQ(*array->values[3].number(), -4.0);
}

TEST(CoreTemplateExpressionTests, EvaluatesInclusiveRangeWithExpressionBounds) {
    drogular::RenderContext context;
    context.set("page", 2);
    context.set("size", 3);

    const auto value = evaluate(
        "[page * size..(page + 1) * size - 1]",
        context
    );
    const auto* range = value.range();

    ASSERT_NE(range, nullptr);
    EXPECT_EQ(range->start, 6);
    EXPECT_EQ(range->end, 8);
    EXPECT_EQ(range->step, 1);
    EXPECT_TRUE(range->upperInclusive);
    EXPECT_EQ(range->materialize(), (std::vector<std::int64_t>{6, 7, 8}));
}

TEST(CoreTemplateExpressionTests, EvaluatesExclusiveRangeWithExpressionStep) {
    drogular::RenderContext context;
    context.set("start", 1);
    context.set("count", 10);
    context.set("stride", 2);

    const auto value = evaluate(
        "[start..<count step stride]",
        context
    );
    const auto* range = value.range();

    ASSERT_NE(range, nullptr);
    EXPECT_FALSE(range->upperInclusive);
    EXPECT_EQ(range->materialize(), (std::vector<std::int64_t>{1, 3, 5, 7, 9}));
}

TEST(CoreTemplateExpressionTests, InfersDescendingRangeStep) {
    drogular::RenderContext context;
    context.set("from", 5);
    context.set("to", 1);

    const auto value = evaluate("[from..to]", context);
    const auto* range = value.range();

    ASSERT_NE(range, nullptr);
    EXPECT_EQ(range->step, -1);
    EXPECT_EQ(range->materialize(), (std::vector<std::int64_t>{5, 4, 3, 2, 1}));
}

TEST(CoreTemplateExpressionTests, RejectsInvalidEvaluatedRangeStep) {
    drogular::RenderContext context;
    context.set("from", 1);
    context.set("to", 10);
    context.set("zero", 0);
    context.set("backward", -2);

    EXPECT_TRUE(evaluate("[from..to step zero]", context).isNull());
    EXPECT_TRUE(evaluate("[from..to step backward]", context).isNull());
}

TEST(CoreTemplateExpressionTests, ParsesRangeSeparatelyFromDottedPaths) {
    const auto result = parse("[start..user.lastPage]");

    ASSERT_TRUE(result);
    const auto* range = std::get_if<RangeExpression>(&result.expression->node);
    ASSERT_NE(range, nullptr);

    const auto* start = std::get_if<VariableExpression>(&range->start->node);
    const auto* end = std::get_if<VariableExpression>(&range->end->node);
    ASSERT_NE(start, nullptr);
    ASSERT_NE(end, nullptr);
    EXPECT_EQ(start->path, "start");
    EXPECT_EQ(end->path, "user.lastPage");
}

TEST(CoreTemplateExpressionTests, ParsesMembershipOperatorsAtComparisonPrecedence) {
    const auto result = parse("role in ['Admin', 'Moderator'] && active");

    ASSERT_TRUE(result);
    const auto* root =
        std::get_if<BinaryExpression>(&result.expression->node);
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->op, BinaryOperator::And);

    const auto* membership =
        std::get_if<BinaryExpression>(&root->left->node);
    ASSERT_NE(membership, nullptr);
    EXPECT_EQ(membership->op, BinaryOperator::In);

    const auto excluded = parse("role not in ['Guest']");
    ASSERT_TRUE(excluded);
    const auto* notIn =
        std::get_if<BinaryExpression>(&excluded.expression->node);
    ASSERT_NE(notIn, nullptr);
    EXPECT_EQ(notIn->op, BinaryOperator::NotIn);
}

TEST(CoreTemplateExpressionTests, EvaluatesMembershipInExpressionLists) {
    drogular::RenderContext context;
    context.set("role", std::string("Admin"));
    context.set("page", 3);

    EXPECT_TRUE(evaluate(
        "role in ['Admin', 'Moderator']",
        context
    ).truthy());
    EXPECT_FALSE(evaluate(
        "role not in ['Admin', 'Moderator']",
        context
    ).truthy());
    EXPECT_TRUE(evaluate("page in [1, 3, 5, 7]", context).truthy());
    EXPECT_FALSE(evaluate("page in [2, 4, 6]", context).truthy());
}

TEST(CoreTemplateExpressionTests, EvaluatesMembershipInJsonArrays) {
    drogular::RenderContext context;
    context.set("role", std::string("Moderator"));

    Json::Value roles(Json::arrayValue);
    roles.append("Admin");
    roles.append("Moderator");
    context.set("roles", roles);

    EXPECT_TRUE(evaluate("role in roles", context).truthy());
    EXPECT_FALSE(evaluate("'Guest' in roles", context).truthy());
    EXPECT_TRUE(evaluate("'Guest' not in roles", context).truthy());
}

TEST(CoreTemplateExpressionTests, EvaluatesMembershipInRangesWithoutMaterializing) {
    drogular::RenderContext context;

    EXPECT_TRUE(evaluate("5 in [1..10]", context).truthy());
    EXPECT_TRUE(evaluate("9 in [1..<10]", context).truthy());
    EXPECT_FALSE(evaluate("10 in [1..<10]", context).truthy());

    EXPECT_TRUE(evaluate("7 in [1..10 step 2]", context).truthy());
    EXPECT_FALSE(evaluate("8 in [1..10 step 2]", context).truthy());

    EXPECT_TRUE(evaluate("4 in [10..0 step -2]", context).truthy());
    EXPECT_FALSE(evaluate("5 in [10..0 step -2]", context).truthy());
}

TEST(CoreTemplateExpressionTests, MembershipRequiresIterableRightOperand) {
    drogular::RenderContext context;

    EXPECT_FALSE(evaluate("1 in 1", context).truthy());
    EXPECT_TRUE(evaluate("1 not in 1", context).truthy());
}

TEST(CoreTemplateExpressionTests, MembershipKeywordsRemainValidVariables) {
    drogular::RenderContext context;
    context.set("in", 3);
    context.set("not", 4);

    EXPECT_TRUE(evaluate("in == 3", context).truthy());
    EXPECT_TRUE(evaluate("not == 4", context).truthy());
}

TEST(CoreTemplateExpressionTests, ReportsIncompleteNotInOperator) {
    const auto result = parse("role not ['Guest']");

    ASSERT_FALSE(result);
    ASSERT_TRUE(result.error.has_value());
    EXPECT_EQ(result.error->message, "Expected 'in' after 'not'");
}

TEST(CoreTemplateExpressionTests, ExposesUniformIterableForListRangeAndJsonArray) {
    drogular::RenderContext context;

    const auto list =
        evaluate("[1, 3, 5]", context).iterable();
    ASSERT_NE(list, nullptr);
    EXPECT_EQ(list->size(), 3);
    ASSERT_TRUE(list->at(1).number().has_value());
    EXPECT_EQ(*list->at(1).number(), 3);

    const auto range =
        evaluate("[2..8 step 2]", context).iterable();
    ASSERT_NE(range, nullptr);
    EXPECT_EQ(range->size(), 4);
    ASSERT_TRUE(range->at(3).number().has_value());
    EXPECT_EQ(*range->at(3).number(), 8);

    Json::Value values(Json::arrayValue);
    values.append("A");
    values.append("B");
    context.set("values", values);
    const auto json =
        evaluate("values", context).iterable();
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->size(), 2);
    ASSERT_TRUE(json->at(0).string().has_value());
    EXPECT_EQ(*json->at(0).string(), "A");
}

TEST(CoreTemplateExpressionTests, RangeIterableDoesNotRequireMaterialization) {
    drogular::RenderContext context;
    const auto range =
        evaluate("[1..1000000000 step 10]", context).iterable();

    ASSERT_NE(range, nullptr);
    EXPECT_EQ(range->size(), 100000000);
    ASSERT_TRUE(range->at(99999999).number().has_value());
    EXPECT_EQ(*range->at(99999999).number(), 999999991);
}

TEST(CoreTemplateExpressionTests, ParsesCollectionMethodCallsIntoAst) {
    const auto result = parse("projects.count() > 0");

    ASSERT_TRUE(result);
    const auto* comparison =
        std::get_if<BinaryExpression>(&result.expression->node);
    ASSERT_NE(comparison, nullptr);
    EXPECT_EQ(comparison->op, BinaryOperator::Greater);

    const auto* call =
        std::get_if<MethodCallExpression>(&comparison->left->node);
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->method, "count");
    EXPECT_TRUE(call->arguments.empty());

    const auto* receiver =
        std::get_if<VariableExpression>(&call->object->node);
    ASSERT_NE(receiver, nullptr);
    EXPECT_EQ(receiver->path, "projects");
}

TEST(CoreTemplateExpressionTests, EvaluatesCollectionCountAndEmptyMethods) {
    drogular::RenderContext context;

    EXPECT_DOUBLE_EQ(*evaluate("[1, 3, 5].count()", context).number(), 3.0);
    EXPECT_DOUBLE_EQ(*evaluate("[1..<10 step 2].count()", context).number(), 5.0);
    EXPECT_TRUE(evaluate("[].empty()", context).truthy());
    EXPECT_FALSE(evaluate("[1..1].empty()", context).truthy());

    Json::Value projects(Json::arrayValue);
    projects.append("A");
    projects.append("B");
    context.set("projects", projects);

    EXPECT_DOUBLE_EQ(*evaluate("projects.count()", context).number(), 2.0);
    EXPECT_FALSE(evaluate("projects.empty()", context).truthy());
}

TEST(CoreTemplateExpressionTests, EvaluatesFirstLastAndContainsMethods) {
    drogular::RenderContext context;

    const auto first =
        evaluate("[10..20 step 5].first()", context);
    const auto last =
        evaluate("[10..20 step 5].last()", context);
    ASSERT_TRUE(first.number().has_value());
    ASSERT_TRUE(last.number().has_value());
    EXPECT_DOUBLE_EQ(*first.number(), 10.0);
    EXPECT_DOUBLE_EQ(*last.number(), 20.0);

    EXPECT_TRUE(evaluate("[1..10].contains(7)", context).truthy());
    EXPECT_FALSE(evaluate("[1..10 step 2].contains(8)", context).truthy());
    EXPECT_TRUE(evaluate("['Admin', 'User'].contains('Admin')", context).truthy());
}

TEST(CoreTemplateExpressionTests, MethodContainsMatchesMembershipSemantics) {
    drogular::RenderContext context;
    context.set("role", std::string("Admin"));

    EXPECT_EQ(
        evaluate("['Admin', 'User'].contains(role)", context).truthy(),
        evaluate("role in ['Admin', 'User']", context).truthy()
    );
    EXPECT_EQ(
        evaluate("[1..<10 step 2].contains(7)", context).truthy(),
        evaluate("7 in [1..<10 step 2]", context).truthy()
    );
}

TEST(CoreTemplateExpressionTests, SupportsMethodAndMemberChaining) {
    drogular::RenderContext context;

    Json::Value projects(Json::arrayValue);
    Json::Value project;
    project["name"] = "Drogular";
    project["roles"] = Json::Value(Json::arrayValue);
    project["roles"].append("Admin");
    project["roles"].append("User");
    projects.append(project);
    context.set("projects", projects);

    const auto name =
        evaluate("projects.first().name", context);
    ASSERT_TRUE(name.string().has_value());
    EXPECT_EQ(*name.string(), "Drogular");
    EXPECT_TRUE(evaluate(
        "projects.first().roles.contains('Admin')",
        context
    ).truthy());
}

TEST(CoreTemplateExpressionTests, EmptyCollectionsReturnNullFirstAndLast) {
    drogular::RenderContext context;

    EXPECT_TRUE(evaluate("[].first()", context).isNull());
    EXPECT_TRUE(evaluate("[].last()", context).isNull());
}

TEST(CoreTemplateExpressionTests, CollectionMethodsValidateArityAndReceiver) {
    drogular::RenderContext context;

    EXPECT_TRUE(evaluate("[1, 2].count(1)", context).isNull());
    EXPECT_TRUE(evaluate("[1, 2].contains()", context).isNull());
    EXPECT_TRUE(evaluate("[1, 2].contains(1, 2)", context).isNull());
    EXPECT_TRUE(evaluate("1.count()", context).isNull());
    EXPECT_TRUE(evaluate("[1, 2].unknown()", context).isNull());
}