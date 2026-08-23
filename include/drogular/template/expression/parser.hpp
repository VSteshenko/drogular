#pragma once

#include <drogular/template/expression/ast.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace drogular::template_expression {

struct ExpressionError {
    std::string message;
    std::size_t position = 0;
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

} // namespace drogular::template_expression