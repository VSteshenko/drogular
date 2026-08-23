#include <drogular/template_parser.hpp>
#include <drogular/template_runtime.hpp>

#include <memory>
#include <unordered_set>

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
    TemplateDiagnostics& diagnostics,
    std::size_t loopDepth
);

NodePtr parseNode(
    const std::vector<Token>& tokens,
    size_t& position,
    TemplateDiagnostics& diagnostics,
    std::size_t loopDepth,
    std::unordered_set<std::string>& localBindings
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

            if (const auto error = validateConditionExpression(token.value)) {
                diagnostics.error(
                    "DGL-TPL-006",
                    "Invalid @if expression: " + error->message,
                    ifPosition + 4 + error->position
                );
            }

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
                    diagnostics,
                    loopDepth
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
                        diagnostics,
                        loopDepth
                    );
            }

            if (position < tokens.size() &&
                tokens[position].type == TokenType::EndIf) {
                ++position;
            } else {
                diagnostics.error(
                    "DGL-TPL-004",
                    "Missing @endif",
                    ifPosition
                );
            }

            return ifNode;
        }

        case TokenType::Let: {
            const auto letPosition = token.position;
            const auto error =
                validateBindingExpression(token.value);
            if (error.has_value()) {
                const auto code = error->message == "Expected binding identifier"
                    ? "DGL-TPL-022"
                    : "DGL-TPL-020";
                diagnostics.error(
                    code,
                    "Invalid @let expression: " + error->message,
                    letPosition + 5 + error->position
                );
                ++position;
                return std::make_shared<TextNode>("");
            }

            const auto declaration =
                parseBindingExpression(token.value);
            if (!declaration.has_value()) {
                ++position;
                return std::make_shared<TextNode>("");
            }

            if (!localBindings.insert(declaration->name).second) {
                diagnostics.error(
                    "DGL-TPL-021",
                    "Duplicate binding '" + declaration->name + "' in the same scope",
                    letPosition
                );
            }

            ++position;
            return std::make_shared<LetNode>(
                declaration->name,
                declaration->expression
            );
        }

        case TokenType::Const: {
            const auto constPosition = token.position;
            const auto error = validateBindingExpression(token.value);
            if (error.has_value()) {
                const auto code = error->message == "Expected binding identifier"
                    ? "DGL-TPL-024"
                    : "DGL-TPL-023";
                diagnostics.error(
                    code,
                    "Invalid @const expression: " + error->message,
                    constPosition + 7 + error->position
                );
                ++position;
                return std::make_shared<TextNode>("");
            }

            const auto declaration = parseBindingExpression(token.value);
            if (!declaration.has_value()) {
                ++position;
                return std::make_shared<TextNode>("");
            }

            if (!localBindings.insert(declaration->name).second) {
                diagnostics.error(
                    "DGL-TPL-021",
                    "Duplicate binding '" + declaration->name + "' in the same scope",
                    constPosition
                );
            }

            ++position;
            return std::make_shared<ConstNode>(
                declaration->name,
                declaration->expression
            );
        }

        case TokenType::Foreach: {
            const auto foreachPosition = token.position;

            if (const auto error = validateForeachExpression(token.value)) {
                diagnostics.error(
                    "DGL-TPL-007",
                    "Invalid @foreach expression: " + error->message,
                    foreachPosition + 9 + error->position
                );
            } else if (const auto expression = parseForeachExpression(token.value);
                expression.has_value() && expression->condition.has_value()
            ) {
                if (const auto error =
                    validateConditionExpression(*expression->condition)
                ) {
                    diagnostics.error(
                        "DGL-TPL-008",
                        "Invalid @foreach where condition: " + error->message,
                        foreachPosition + 9 + expression->conditionPosition + error->position
                    );
                }
            }

            auto foreachNode =
                std::make_shared<ForeachNode>(token.value);

            ++position;

            foreachNode->body() =
                parseUntil(
                    tokens,
                    position,
                    {
                        TokenType::Empty,
                        TokenType::EndForeach
                    },
                    diagnostics,
                    loopDepth + 1
                );

            if (position < tokens.size() &&
                tokens[position].type == TokenType::Empty
            ) {
                ++position;
                foreachNode->emptyBranch() =
                    parseUntil(
                        tokens,
                        position,
                        { TokenType::EndForeach },
                        diagnostics,
                        loopDepth
                    );
            }

            if (position < tokens.size() &&
                tokens[position].type == TokenType::EndForeach
            ) {
                ++position;
            } else {
                diagnostics.error(
                    "DGL-TPL-005",
                    "Missing @endforeach",
                    foreachPosition
                );
            }

            return foreachNode;
        }

        case TokenType::Break:
            if (loopDepth == 0) {
                diagnostics.error(
                    "DGL-TPL-010",
                    "Unexpected @break outside @foreach",
                    token.position
                );
            }
            ++position;
            return std::make_shared<BreakNode>();

        case TokenType::Continue:
            if (loopDepth == 0) {
                diagnostics.error(
                    "DGL-TPL-011",
                    "Unexpected @continue outside @foreach",
                    token.position
                );
            }
            ++position;
            return std::make_shared<ContinueNode>();

        case TokenType::Else:
        case TokenType::EndIf:
        case TokenType::Empty:
        case TokenType::EndForeach:
            return nullptr;
    }

    return nullptr;
}

std::vector<NodePtr> parseUntil(
    const std::vector<Token>& tokens,
    size_t& position,
    const std::vector<TokenType>& stopTokens,
    TemplateDiagnostics& diagnostics,
    std::size_t loopDepth
) {
    std::vector<NodePtr> nodes;
    std::unordered_set<std::string> localBindings;

    while (position < tokens.size()) {
        if (isStopToken(tokens[position].type, stopTokens)) {
            break;
        }

        auto node = parseNode(
            tokens, position, diagnostics, loopDepth, localBindings
        );

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
            diagnostics,
            0
        );

    while (position < tokens.size()) {
        const auto& token = tokens[position];

        if (token.type == TokenType::Else) {
            diagnostics.error(
                "DGL-TPL-001",
                "Unexpected @else",
                token.position
            );
        } else if (token.type == TokenType::EndIf) {
            diagnostics.error(
                "DGL-TPL-002",
                "Unexpected @endif",
                token.position
            );
        } else if (token.type == TokenType::Empty) {
            diagnostics.error(
                "DGL-TPL-009",
                "Unexpected @empty",
                token.position
            );
        } else if (token.type == TokenType::EndForeach) {
            diagnostics.error(
                "DGL-TPL-003",
                "Unexpected @endforeach",
                token.position
            );
        }

        ++position;
    }

    return nodes;
}

} // namespace drogular::template_compiler