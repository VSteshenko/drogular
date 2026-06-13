#pragma once

#include <drogular/template_ast.hpp>
#include <drogular/template_tokenizer.hpp>
#include <drogular/template_diagnostics.hpp>

#include <vector>

namespace drogular::template_compiler {

/**
 * Parses template tokens into an AST.
 */
std::vector<NodePtr> parse(const std::vector<Token>& tokens);

/**
 * Parses template tokens into an AST and records diagnostics.
 */
std::vector<NodePtr> parse(
    const std::vector<Token>& tokens,
    TemplateDiagnostics& diagnostics
);

} // namespace drogular::template_compiler
