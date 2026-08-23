#pragma once

#include <json/json.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace drogular {

class RenderContext;

namespace template_expression {

struct ExpressionError {
    std::string message;
    std::size_t position = 0;
};

class ExpressionValue;
struct ExpressionArray;

/**
 * Evaluated integer range.
 *
 * The upper bound may be inclusive (`..`) or exclusive (`..<`). The step is
 * always non-zero and has a direction compatible with start/end.
 */
struct ExpressionRange {
    std::int64_t start = 0;
    std::int64_t end = 0;
    std::int64_t step = 1;
    bool upperInclusive = true;

    [[nodiscard]] std::vector<std::int64_t> materialize() const;
};

/**
 * Runtime value produced by the template expression engine.
 *
 * Json::Value is intentionally retained as a storage alternative so existing
 * RenderContext JSON objects and arrays can flow through the expression engine
 * without conversion. Native arrays and ranges provide expression-owned
 * iterable values for list/range literals.
 */
class ExpressionValue {
public:
    using ArrayPtr = std::shared_ptr<const ExpressionArray>;
    using Storage = std::variant<
        std::monostate,
        bool,
        double,
        std::string,
        Json::Value,
        ArrayPtr,
        ExpressionRange
    >;

    ExpressionValue() = default;
    explicit ExpressionValue(bool value);
    explicit ExpressionValue(double value);
    explicit ExpressionValue(std::string value);
    explicit ExpressionValue(Json::Value value);
    explicit ExpressionValue(ArrayPtr value);
    explicit ExpressionValue(ExpressionRange value);

    [[nodiscard]] bool truthy() const;
    [[nodiscard]] bool isNull() const;
    [[nodiscard]] std::optional<double> number() const;
    [[nodiscard]] std::optional<std::string> string() const;
    [[nodiscard]] std::optional<bool> boolean() const;
    [[nodiscard]] ArrayPtr array() const;
    [[nodiscard]] const ExpressionRange* range() const;
    [[nodiscard]] const Storage& storage() const noexcept;

private:
    Storage value_;
};

struct ExpressionArray {
    std::vector<ExpressionValue> values;
};

enum class UnaryOperator {
    Not,
    Negate
};

enum class BinaryOperator {
    Add,
    Subtract,
    Multiply,
    Divide,
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

struct ListExpression {
    std::vector<ExpressionPtr> elements;
};

struct RangeExpression {
    ExpressionPtr start;
    ExpressionPtr end;
    ExpressionPtr step;
    bool upperInclusive = true;
};

using ExpressionNode = std::variant<
    LiteralExpression,
    VariableExpression,
    UnaryExpression,
    BinaryExpression,
    ListExpression,
    RangeExpression
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
