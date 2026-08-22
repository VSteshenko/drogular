#pragma once

#include <drogular/component.hpp>
#include <drogular/template_expression.hpp>

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

/**
 * Resolves a template expression into raw text.
 */
std::optional<std::string> resolveRawVariable(
    std::string_view expression,
    const RenderContext& context
);

using ConditionExpressionError = template_expression::ExpressionError;

struct ForeachExpression {
    std::string variable;
    std::string collection;
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
 *   item in items where item.active
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

} // namespace detail

} // namespace drogular::template_compiler