#pragma once

#include <drogular/template/expression/ast.hpp>

#include <string_view>

namespace drogular {

class RenderContext;

namespace template_expression {

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