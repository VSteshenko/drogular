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
            const auto parsedCondition = template_expression::parse(token.value);

            if (!parsedCondition) {
                diagnostics.error(
                    "DGL-TPL-006",
                    "Invalid @if expression: " + parsedCondition.error->message,
                    ifPosition + 4 + parsedCondition.error->position
                );
            }

            auto ifNode = std::make_shared<IfNode>(
                token.value,
                parsedCondition.expression
            );

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
                declaration->expression,
                declaration->compiledExpression
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
                declaration->expression,
                declaration->compiledExpression
            );
        }

        case TokenType::Switch: {
            const auto switchPosition = token.position;
            const auto parsedSwitch = template_expression::parse(token.value);
            if (!parsedSwitch) {
                diagnostics.error(
                    "DGL-TPL-028",
                    "Invalid @switch expression: " + parsedSwitch.error->message,
                    switchPosition + 8 + parsedSwitch.error->position
                );
            }

            auto switchNode = std::make_shared<SwitchNode>(
                token.value,
                parsedSwitch.expression
            );
            ++position;
            bool hasDefault = false;

            while (position < tokens.size() &&
                   tokens[position].type != TokenType::EndSwitch) {
                if (tokens[position].type == TokenType::Case) {
                    const auto caseToken = tokens[position];
                    template_expression::ExpressionPtr compiledExpressions;
                    if (caseToken.value.empty()) {
                        diagnostics.error(
                            "DGL-TPL-029",
                            "Invalid @case expression: Expected value",
                            caseToken.position
                        );
                    } else {
                        const auto parsed = template_expression::parse(
                            "[" + caseToken.value + "]"
                        );
                        compiledExpressions = parsed.expression;
                        if (!parsed) {
                            diagnostics.error(
                                "DGL-TPL-029",
                                "Invalid @case expression: " + parsed.error->message,
                                caseToken.position + 6 +
                                    (parsed.error->position > 0
                                        ? parsed.error->position - 1
                                        : 0)
                            );
                        }
                    }

                    ++position;
                    SwitchCase switchCase;
                    switchCase.expressions = caseToken.value;
                    switchCase.compiledExpressions = std::move(compiledExpressions);
                    switchCase.body = parseUntil(
                        tokens,
                        position,
                        {
                            TokenType::Case,
                            TokenType::Default,
                            TokenType::EndSwitch },
                        diagnostics,
                        loopDepth
                    );
                    switchNode->cases().push_back(std::move(switchCase));
                    continue;
                }

                if (tokens[position].type == TokenType::Default) {
                    const auto defaultPosition = tokens[position].position;
                    const bool duplicateDefault = hasDefault;
                    if (duplicateDefault) {
                        diagnostics.error(
                            "DGL-TPL-027",
                            "Duplicate @default",
                            defaultPosition
                        );
                    }
                    hasDefault = true;
                    ++position;
                    auto branch = parseUntil(
                        tokens,
                        position,
                        { TokenType::Case, TokenType::Default, TokenType::EndSwitch },
                        diagnostics,
                        loopDepth
                    );
                    if (!duplicateDefault) {
                        switchNode->defaultBranch() = std::move(branch);
                    }
                    continue;
                }

                if (tokens[position].type == TokenType::Text) {
                    bool whitespaceOnly = true;
                    for (const auto ch : tokens[position].value) {
                        if (!std::isspace(static_cast<unsigned char>(ch))) {
                            whitespaceOnly = false;
                            break;
                        }
                    }
                    if (whitespaceOnly) {
                        ++position;
                        continue;
                    }
                }

                diagnostics.error(
                    "DGL-TPL-025",
                    "Expected @case or @default inside @switch",
                    tokens[position].position
                );
                ++position;
            }

            if (position < tokens.size() &&
                tokens[position].type == TokenType::EndSwitch) {
                ++position;
            } else {
                diagnostics.error(
                    "DGL-TPL-030",
                    "Missing @endswitch",
                    switchPosition
                );
            }

            return switchNode;
        }

        case TokenType::Foreach: {
            const auto foreachPosition = token.position;
            std::optional<ForeachExpression> expression;
            template_expression::ExpressionPtr collectionExpression;
            template_expression::ExpressionPtr conditionExpression;

            if (const auto error = validateForeachExpression(token.value)) {
                diagnostics.error(
                    "DGL-TPL-007",
                    "Invalid @foreach expression: " + error->message,
                    foreachPosition + 9 + error->position
                );
            } else {
                expression = parseForeachExpression(token.value);
                if (expression.has_value()) {
                    collectionExpression = expression->collectionExpression;
                    conditionExpression = expression->conditionExpression;
                    if (expression->condition.has_value() &&
                        conditionExpression == nullptr
                    ) {
                        const auto parsedCondition =
                            template_expression::parse(*expression->condition);
                        diagnostics.error(
                            "DGL-TPL-008",
                            "Invalid @foreach where condition: " +
                                parsedCondition.error->message,
                            foreachPosition + 9 + expression->conditionPosition +
                                parsedCondition.error->position
                        );
                    }
                }
            }

            auto foreachNode = std::make_shared<ForeachNode>(
                token.value,
                expression.has_value() ? expression->variable : std::string{},
                expression.has_value() ? expression->collection : std::string{},
                std::move(collectionExpression),
                expression.has_value() ? expression->condition : std::nullopt,
                std::move(conditionExpression)
            );

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

        case TokenType::Case:
            diagnostics.error(
                "DGL-TPL-025",
                "Unexpected @case",
                token.position
            );
            ++position;
            return std::make_shared<TextNode>("");

        case TokenType::Default:
            diagnostics.error(
                "DGL-TPL-026",
                "Unexpected @default",
                token.position
            );
            ++position;
            return std::make_shared<TextNode>("");

        case TokenType::EndSwitch:
            diagnostics.error(
                "DGL-TPL-031",
                "Unexpected @endswitch",
                token.position
            );
            ++position;
            return std::make_shared<TextNode>("");

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