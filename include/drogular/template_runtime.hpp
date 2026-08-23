#pragma once

#include <drogular/component.hpp>
#include <drogular/template/expression/expression.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <json/json.h>

namespace drogular::template_compiler {

/**
 * Resolves a template expression into escaped HTML text.
 */
std::optional<std::string> resolveVariable(
    std::string_view expression,
    const RenderContext& context
);

std::optional<std::string> resolveVariable(
    std::string_view expression,
    const template_expression::BindingContext& context
);

/**
 * Resolves a template expression into raw text.
 */
std::optional<std::string> resolveRawVariable(
    std::string_view expression,
    const RenderContext& context
);

std::optional<std::string> resolveRawVariable(
    std::string_view expression,
    const template_expression::BindingContext& context
);

using ConditionExpressionError = template_expression::ExpressionError;

struct LetExpression {
    std::string name;
    std::string expression;
    std::size_t expressionPosition = 0;
};

struct LetExpressionError {
    std::string message;
    std::size_t position = 0;
};

/** Parses a @let declaration in the form `name = expression`. */
std::optional<LetExpression> parseLetExpression(std::string_view expression);

/** Validates @let declaration syntax and the right-hand expression. */
std::optional<LetExpressionError> validateLetExpression(
    std::string_view expression
);


struct ForeachExpression {
    std::string variable;
    std::string collection;
    std::size_t collectionPosition = 0;
    std::optional<std::string> condition;
    std::size_t conditionPosition = 0;
};

struct ForeachExpressionError {
    std::string message;
    std::size_t position = 0;
};

/**
 * Parses a @foreach expression.
 *
 * Supported forms:
 *   item in items
 *   item in [1..10]
 *   item in expression where item.active
 */
std::optional<ForeachExpression> parseForeachExpression(
    std::string_view expression
);

/**
 * Validates the structure of a @foreach expression.
 */
std::optional<ForeachExpressionError> validateForeachExpression(
    std::string_view expression
);

/**
 * Validates the syntax of a template condition.
 *
 * Returns the first syntax error, if any.
 */
std::optional<ConditionExpressionError> validateConditionExpression(
    std::string_view expression
);

/**
 * Evaluates a template condition.
 *
 * Invalid expressions evaluate to false. Template compilation reports
 * condition syntax errors through TemplateDiagnostics.
 */
bool evaluateCondition(
    std::string_view expression,
    const RenderContext& context
);

bool evaluateCondition(
    std::string_view expression,
    const template_expression::BindingContext& context
);

/**
 * Resolves a template expression into Json::Value.
 *
 * Supports direct Json values and dotted paths:
 *
 * user
 * user.name
 * todo.author.profile.name
 */
std::optional<Json::Value> resolveJsonValue(
    std::string_view expression,
    const RenderContext& context
);

namespace detail {

/**
 * Creates and stores metadata for a @foreach iteration.
 *
 * Nested loops expose the nearest outer loop through `parent` and use
 * zero-based `depth` (outermost loop = 0).
 */
void setLoopMetadata(
    RenderContext& childContext,
    const RenderContext& parentContext,
    std::size_t index,
    std::size_t count
);

/** Stores an ExpressionValue in a render context without losing its type. */
void setExpressionValue(
    RenderContext& context,
    std::string key,
    template_expression::ExpressionValue value
);

} // namespace detail

} // namespace drogular::template_compiler