#pragma once

#include <drogular/template_diagnostics.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace drogular::template_compiler {

enum class TokenType {
    Text,
    Variable,
    RawVariable,
    If,
    Else,
    EndIf,
    Foreach,
    Let,
    Empty,
    Break,
    Continue,
    EndForeach,
    ComponentTag
};

struct Token {
    TokenType type;
    std::string value;
    size_t position = 0;
};

/**
 * Converts template text into tokens.
 */
std::vector<Token> tokenize(std::string_view html);

/**
 * Converts template text into tokens and records tokenizer diagnostics.
 */
std::vector<Token> tokenize(
    std::string_view html,
    TemplateDiagnostics& diagnostics
);

} // namespace drogular::template_compiler