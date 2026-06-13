#include <drogular/template_parser.hpp>

#include <memory>

namespace drogular::template_compiler {

namespace {

bool isStopToken(
    TokenType type,
    const std::vector<TokenType>& stopTokens
) {
    for (const auto stopToken : stopTokens) {
        if (type == stopToken) {
            return true;
        }
    }

    return false;
}

std::vector<NodePtr> parseUntil(
    const std::vector<Token>& tokens,
    size_t& position,
    const std::vector<TokenType>& stopTokens,
    TemplateDiagnostics& diagnostics
);

NodePtr parseNode(
    const std::vector<Token>& tokens,
    size_t& position,
    TemplateDiagnostics& diagnostics
) {
    const auto& token = tokens[position];

    switch (token.type) {
        case TokenType::Text:
            ++position;
            return std::make_shared<TextNode>(token.value);

        case TokenType::Variable:
            ++position;
            return std::make_shared<VariableNode>(token.value);

        case TokenType::RawVariable:
            ++position;
            return std::make_shared<RawVariableNode>(token.value);

        case TokenType::ComponentTag:
            ++position;
            return std::make_shared<ComponentNode>(token.value);

        case TokenType::If: {
            const auto ifPosition = token.position;

            auto ifNode =
                std::make_shared<IfNode>(token.value);

            ++position;

            ifNode->trueBranch() =
                parseUntil(
                    tokens,
                    position,
        {
                        TokenType::Else,
                        TokenType::EndIf
                    },
                    diagnostics
                );

            if (position < tokens.size() &&
                tokens[position].type == TokenType::Else) {
                ++position;

                ifNode->falseBranch() =
                    parseUntil(
                        tokens,
                        position,
            {
                            TokenType::EndIf
                        },
                        diagnostics
                    );
            }

            if (position < tokens.size() &&
                tokens[position].type == TokenType::EndIf) {
                ++position;
            } else {
                diagnostics.addError(
                    "Unclosed @if",
                    ifPosition
                );
            }

            return ifNode;
        }

        case TokenType::Foreach: {
            const auto foreachPosition = token.position;

            auto foreachNode =
                std::make_shared<ForeachNode>(token.value);

            ++position;

            foreachNode->body() =
                parseUntil(
                    tokens,
                    position,
                    {
                        TokenType::EndForeach
                    },
                    diagnostics
                );

            if (position < tokens.size() &&
                tokens[position].type == TokenType::EndForeach) {
                ++position;
                } else {
                    diagnostics.addError(
                        "Unclosed @foreach",
                        foreachPosition
                    );
            }

            return foreachNode;
        }

        case TokenType::Else:
        case TokenType::EndIf:
        case TokenType::EndForeach:
            return nullptr;
    }

    return nullptr;
}

std::vector<NodePtr> parseUntil(
    const std::vector<Token>& tokens,
    size_t& position,
    const std::vector<TokenType>& stopTokens,
    TemplateDiagnostics& diagnostics
) {
    std::vector<NodePtr> nodes;

    while (position < tokens.size()) {
        if (isStopToken(tokens[position].type, stopTokens)) {
            break;
        }

        auto node = parseNode(tokens, position, diagnostics);

        if (node != nullptr) {
            nodes.push_back(std::move(node));
        } else {
            break;
        }
    }

    return nodes;
}

} // namespace

std::vector<NodePtr> parse(const std::vector<Token>& tokens) {
    TemplateDiagnostics diagnostics;

    return parse(tokens, diagnostics);
}

std::vector<NodePtr> parse(
    const std::vector<Token>& tokens,
    TemplateDiagnostics& diagnostics
) {
    size_t position = 0;

    auto nodes =
        parseUntil(
            tokens,
            position,
            {},
            diagnostics
        );

    while (position < tokens.size()) {
        const auto& token = tokens[position];

        if (token.type == TokenType::Else) {
            diagnostics.addError("Unexpected @else", token.position);
        } else if (token.type == TokenType::EndIf) {
            diagnostics.addError("Unexpected @endif", token.position);
        } else if (token.type == TokenType::EndForeach) {
            diagnostics.addError("Unexpected @endforeach", token.position);
        }

        ++position;
    }

    return nodes;
}

} // namespace drogular::template_compiler