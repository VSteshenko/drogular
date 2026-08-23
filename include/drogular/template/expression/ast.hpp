#pragma once

#include <drogular/template/expression/value.hpp>

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace drogular::template_expression {

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
    In,
    NotIn,
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

struct MemberAccessExpression {
    ExpressionPtr object;
    std::string member;
};

struct CallExpression {
    std::string function;
    std::vector<ExpressionPtr> arguments;
};

struct MethodCallExpression {
    ExpressionPtr object;
    std::string method;
    std::vector<ExpressionPtr> arguments;
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
    MemberAccessExpression,
    CallExpression,
    MethodCallExpression,
    RangeExpression
>;

struct Expression {
    ExpressionNode node;
};

} // namespace drogular::template_expression