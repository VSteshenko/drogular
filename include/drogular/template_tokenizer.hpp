#pragma once

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

} // namespace drogular::template_compiler
