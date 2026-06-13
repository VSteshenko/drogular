#pragma once

#include <drogular/template_ast.hpp>
#include <drogular/template_tokenizer.hpp>

#include <vector>

namespace drogular::template_compiler {

/**
 * Parses template tokens into an AST.
 */
std::vector<NodePtr> parse(const std::vector<Token>& tokens);

} // namespace drogular::template_compiler
