#pragma once

#include <json/json.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace drogular {

class RenderContext;

namespace template_expression {

struct ExpressionError {
    std::string message;
    std::size_t position = 0;
};

/**
 * Runtime value produced by the template expression engine.
 *
 * Json::Value is intentionally retained as a storage alternative so existing
 * RenderContext JSON objects and arrays can flow through the expression engine
 * without conversion. Dedicated collection/range value types can be added on
 * top of this abstraction without changing template directives.
 */
class ExpressionValue {
public:
    using Storage = std::variant<
        std::monostate,
        bool,
        double,
        std::string,
        Json::Value
    >;

    ExpressionValue() = default;
    explicit ExpressionValue(bool value);
    explicit ExpressionValue(double value);
    explicit ExpressionValue(std::string value);
    explicit ExpressionValue(Json::Value value);

    [[nodiscard]] bool truthy() const;
    [[nodiscard]] bool isNull() const;
    [[nodiscard]] std::optional<double> number() const;
    [[nodiscard]] std::optional<std::string> string() const;
    [[nodiscard]] std::optional<bool> boolean() const;
    [[nodiscard]] const Storage& storage() const noexcept;

private:
    Storage value_;
};

enum class UnaryOperator {
    Not
};

enum class BinaryOperator {
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    And,
    Or
};

struct Expression;
using ExpressionPtr = std::shared_ptr<const Expression>;

struct LiteralExpression {
    ExpressionValue value;
};

struct VariableExpression {
    std::string path;
};

struct UnaryExpression {
    UnaryOperator op = UnaryOperator::Not;
    ExpressionPtr operand;
};

struct BinaryExpression {
    BinaryOperator op = BinaryOperator::Equal;
    ExpressionPtr left;
    ExpressionPtr right;
};

using ExpressionNode = std::variant<
    LiteralExpression,
    VariableExpression,
    UnaryExpression,
    BinaryExpression
>;

struct Expression {
    ExpressionNode node;
};

struct ParseResult {
    ExpressionPtr expression;
    std::optional<ExpressionError> error;

    [[nodiscard]] explicit operator bool() const noexcept {
        return expression != nullptr && !error.has_value();
    }
};

/** Parses a template expression into an immutable expression AST. */
ParseResult parse(std::string_view source);

/** Evaluates a previously parsed expression AST. */
ExpressionValue evaluate(
    const Expression& expression,
    const RenderContext& context
);

/** Parses and evaluates an expression. Invalid expressions return null. */
ExpressionValue evaluate(
    std::string_view source,
    const RenderContext& context
);

/** Resolves a RenderContext value or dotted JSON path into ExpressionValue. */
ExpressionValue resolve(
    std::string_view path,
    const RenderContext& context
);

} // namespace template_expression
} // namespace drogular