#include <drogular/template_parser.hpp>

#include <memory>

namespace drogular::template_compiler {

namespace {

std::vector<NodePtr> parseNodes(
    const std::vector<Token>& tokens,
    size_t& position,
    TokenType stopToken
) {
    std::vector<NodePtr> nodes;

    while (position < tokens.size()) {
        const auto& token = tokens[position];

        if (token.type == stopToken) {
            ++position;
            break;
        }

        switch (token.type) {
            case TokenType::Text:
                nodes.push_back(
                    std::make_shared<TextNode>(token.value)
                );
                ++position;
                break;

            case TokenType::Variable:
                nodes.push_back(
                    std::make_shared<VariableNode>(token.value)
                );
                ++position;
                break;

            case TokenType::RawVariable:
                nodes.push_back(
                    std::make_shared<RawVariableNode>(token.value)
                );
                ++position;
                break;

            case TokenType::ComponentTag:
                nodes.push_back(
                    std::make_shared<ComponentNode>(token.value)
                );
                ++position;
                break;

            case TokenType::If: {
                auto ifNode =
                    std::make_shared<IfNode>(token.value);

                ++position;

                while (position < tokens.size()) {
                    if (tokens[position].type == TokenType::Else) {
                        ++position;
                        ifNode->falseBranch() =
                            parseNodes(
                                tokens,
                                position,
                                TokenType::EndIf
                            );
                        break;
                    }

                    if (tokens[position].type == TokenType::EndIf) {
                        ++position;
                        break;
                    }

                    ifNode->trueBranch().push_back(
                        parseNodes(
                            tokens,
                            position,
                            TokenType::Else
                        )[0]
                    );
                }

                nodes.push_back(ifNode);
                break;
            }

            case TokenType::Foreach: {
                auto foreachNode =
                    std::make_shared<ForeachNode>(token.value);

                ++position;

                foreachNode->body() =
                    parseNodes(
                        tokens,
                        position,
                        TokenType::EndForeach
                    );

                nodes.push_back(foreachNode);
                break;
            }

            case TokenType::Else:
            case TokenType::EndIf:
            case TokenType::EndForeach:
                return nodes;
        }
    }

    return nodes;
}

} // namespace

std::vector<NodePtr> parse(const std::vector<Token>& tokens) {
    size_t position = 0;

    std::vector<NodePtr> nodes;

    while (position < tokens.size()) {
        const auto& token = tokens[position];

        switch (token.type) {
            case TokenType::Text:
                nodes.push_back(
                    std::make_shared<TextNode>(token.value)
                );
                ++position;
                break;

            case TokenType::Variable:
                nodes.push_back(
                    std::make_shared<VariableNode>(token.value)
                );
                ++position;
                break;

            case TokenType::RawVariable:
                nodes.push_back(
                    std::make_shared<RawVariableNode>(token.value)
                );
                ++position;
                break;

            case TokenType::ComponentTag:
                nodes.push_back(
                    std::make_shared<ComponentNode>(token.value)
                );
                ++position;
                break;

            case TokenType::If: {
                auto ifNode =
                    std::make_shared<IfNode>(token.value);

                ++position;

                ifNode->trueBranch() =
                    parseNodes(
                        tokens,
                        position,
                        TokenType::EndIf
                    );

                nodes.push_back(ifNode);
                break;
            }

            case TokenType::Foreach: {
                auto foreachNode =
                    std::make_shared<ForeachNode>(token.value);

                ++position;

                foreachNode->body() =
                    parseNodes(
                        tokens,
                        position,
                        TokenType::EndForeach
                    );

                nodes.push_back(foreachNode);
                break;
            }

            case TokenType::Else:
            case TokenType::EndIf:
            case TokenType::EndForeach:
                ++position;
                break;
        }
    }

    return nodes;
}

} // namespace drogular::template_compiler
