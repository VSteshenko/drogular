#pragma once

#include <drogular/template/expression/ast.hpp>
#include <drogular/template/expression/binding_context.hpp>

#include <string_view>

namespace drogular {

class RenderContext;

namespace template_expression {

/** Evaluates a previously parsed expression AST against lexical bindings. */
ExpressionValue evaluate(
    const Expression& expression,
    const BindingContext& context
);

/** Compatibility overload using an empty lexical scope over RenderContext. */
ExpressionValue evaluate(
    const Expression& expression,
    const RenderContext& context
);

/** Parses and evaluates an expression. Invalid expressions return null. */
ExpressionValue evaluate(
    std::string_view source,
    const BindingContext& context
);

/** Compatibility overload using an empty lexical scope over RenderContext. */
ExpressionValue evaluate(
    std::string_view source,
    const RenderContext& context
);

/** Resolves lexical bindings first, then falls back to RenderContext. */
ExpressionValue resolve(
    std::string_view path,
    const BindingContext& context
);

/** Compatibility overload using an empty lexical scope over RenderContext. */
ExpressionValue resolve(
    std::string_view path,
    const RenderContext& context
);

} // namespace template_expression
} // namespace drogular