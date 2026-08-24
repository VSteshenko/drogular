#include <drogular/template_tokenizer.hpp>

#include <cctype>

namespace drogular::template_compiler {

namespace {

bool startsWith(
    std::string_view text,
    size_t position,
    std::string_view value
) {
    return text.substr(position, value.size()) == value;
}

bool isComponentTagStart(
    std::string_view html,
    size_t position
) {
    if (position + 1 >= html.size()) {
        return false;
    }

    if (html[position] != '<') {
        return false;
    }

    return std::isupper(
        static_cast<unsigned char>(html[position + 1])
    );
}

size_t findComponentTagEnd(
    std::string_view html,
    size_t position
) {
    return html.find('>', position);
}

size_t findDirectiveExpressionEnd(
    std::string_view html,
    size_t position
) {
    size_t depth = 1;
    char quote = '\0';
    bool escaped = false;

    for (size_t i = position; i < html.size(); ++i) {
        const auto ch = html[i];

        if (quote != '\0') {
            if (escaped) {
                escaped = false;
                continue;
            }
            if (ch == '\\') {
                escaped = true;
                continue;
            }
            if (ch == quote) quote = '\0';
            continue;
        }

        if (ch == '\'' || ch == '"') {
            quote = ch;
            continue;
        }
        if (ch == '(') {
            ++depth;
            continue;
        }
        if (ch == ')' && --depth == 0) {
            return i;
        }
    }

    return std::string_view::npos;
}

} // namespace

std::vector<Token> tokenize(std::string_view html) {
    TemplateDiagnostics diagnostics;
    return tokenize(html, diagnostics);
}

std::vector<Token> tokenize(
    std::string_view html,
    TemplateDiagnostics& diagnostics
) {
    std::vector<Token> tokens;
    size_t position = 0;
    size_t textStart = 0;

    const auto flushText = [&]() {
        if (textStart < position) {
            tokens.push_back({
                .type = TokenType::Text,
                .value = std::string(
                    html.substr(textStart, position - textStart)
                ),
                .position = textStart
            });
        }
    };

    while (position < html.size()) {
        if (startsWith(html, position, "{{{")) {
            flushText();

            const auto end =
                html.find("}}}", position + 3);

            if (end == std::string_view::npos) {
                tokens.push_back({
                    .type = TokenType::Text,
                    .value = std::string(html.substr(position)),
                    .position = position
                });
                position = html.size();
                textStart = position;
                break;
            }

            tokens.push_back({
                .type = TokenType::RawVariable,
                .value = std::string(
                    html.substr(position + 3, end - position - 3)
                ),
                .position = position
            });

            position = end + 3;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "{{")) {
            flushText();

            const auto end =
                html.find("}}", position + 2);

            if (end == std::string_view::npos) {
                tokens.push_back({
                    .type = TokenType::Text,
                    .value = std::string(html.substr(position)),
                    .position = position
                });
                position = html.size();
                textStart = position;
                break;
            }

            tokens.push_back({
                .type = TokenType::Variable,
                .value = std::string(
                    html.substr(position + 2, end - position - 2)
                ),
                .position = position
            });

            position = end + 2;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@if(")) {
            flushText();

            const auto end =
                findDirectiveExpressionEnd(html, position + 4);

            if (end == std::string_view::npos) {
                diagnostics.error(
                    "DGL-TPL-006",
                    "Invalid @if expression: Missing closing ')'",
                    position
                );
                tokens.push_back({
                    .type = TokenType::Text,
                    .value = std::string(html.substr(position)),
                    .position = position
                });
                break;
            }

            tokens.push_back({
                .type = TokenType::If,
                .value = std::string(
                    html.substr(position + 4, end - position - 4)
                ),
                .position = position
            });

            position = end + 1;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@else")) {
            flushText();

            tokens.push_back({
                .type = TokenType::Else,
                .value = "",
                .position = position
            });

            position += 5;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@endif")) {
            flushText();

            tokens.push_back({
                .type = TokenType::EndIf,
                .value = "",
                .position = position
            });

            position += 6;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@let(")) {
            flushText();

            const auto end =
                findDirectiveExpressionEnd(html, position + 5);

            if (end == std::string_view::npos) {
                diagnostics.error(
                    "DGL-TPL-020",
                    "Invalid @let expression: Missing closing ')'",
                    position
                );
                tokens.push_back({
                    .type = TokenType::Text,
                    .value = std::string(html.substr(position)),
                    .position = position
                });
                break;
            }

            tokens.push_back({
                .type = TokenType::Let,
                .value = std::string(
                    html.substr(position + 5, end - position - 5)
                ),
                .position = position
            });

            position = end + 1;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@const(")) {
            flushText();

            const auto end =
                findDirectiveExpressionEnd(html, position + 7);

            if (end == std::string_view::npos) {
                diagnostics.error(
                    "DGL-TPL-023",
                    "Invalid @const expression: Missing closing ')'",
                    position
                );
                tokens.push_back({
                    .type = TokenType::Text,
                    .value = std::string(html.substr(position)),
                    .position = position
                });
                break;
            }

            tokens.push_back({
                .type = TokenType::Const,
                .value = std::string(
                    html.substr(position + 7, end - position - 7)
                ),
                .position = position
            });

            position = end + 1;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@switch(")) {
            flushText();

            const auto end =
                findDirectiveExpressionEnd(html, position + 8);

            if (end == std::string_view::npos) {
                diagnostics.error(
                    "DGL-TPL-028",
                    "Invalid @switch expression: Missing closing ')'",
                    position
                );
                tokens.push_back({
                    .type = TokenType::Text,
                    .value = std::string(html.substr(position)),
                    .position = position
                });
                break;
            }

            tokens.push_back({
                .type = TokenType::Switch,
                .value = std::string(
                    html.substr(position + 8, end - position - 8)
                ),
                .position = position
            });

            position = end + 1;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@case(")) {
            flushText();

            const auto end =
                findDirectiveExpressionEnd(html, position + 6);

            if (end == std::string_view::npos) {
                diagnostics.error(
                    "DGL-TPL-029",
                    "Invalid @case expression: Missing closing ')'",
                    position
                );
                tokens.push_back({
                    .type = TokenType::Text,
                    .value = std::string(html.substr(position)),
                    .position = position
                });
                break;
            }

            tokens.push_back({
                .type = TokenType::Case,
                .value = std::string(
                    html.substr(position + 6, end - position - 6)
                ),
                .position = position
            });

            position = end + 1;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@default")) {
            flushText();
            tokens.push_back({
                .type = TokenType::Default,
                .value = "",
                .position = position
            });

            position += 8;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@endswitch")) {
            flushText();
            tokens.push_back({
                .type = TokenType::EndSwitch,
                .value = "",
                .position = position
            });

            position += 10;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@foreach(")) {
            flushText();

            const auto end =
                findDirectiveExpressionEnd(html, position + 9);

            if (end == std::string_view::npos) {
                diagnostics.error(
                    "DGL-TPL-007",
                    "Invalid @foreach expression: Missing closing ')'",
                    position
                );
                tokens.push_back({
                    .type = TokenType::Text,
                    .value = std::string(html.substr(position)),
                    .position = position
                });
                break;
            }

            tokens.push_back({
                .type = TokenType::Foreach,
                .value = std::string(
                    html.substr(position + 9, end - position - 9)
                ),
                .position = position
            });

            position = end + 1;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@empty")) {
            flushText();

            tokens.push_back({
                .type = TokenType::Empty,
                .value = "",
                .position = position
            });

            position += 6;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@break")) {
            flushText();

            tokens.push_back({
                .type = TokenType::Break,
                .value = "",
                .position = position
            });

            position += 6;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@continue")) {
            flushText();

            tokens.push_back({
                .type = TokenType::Continue,
                .value = "",
                .position = position
            });

            position += 9;
            textStart = position;

            continue;
        }

        if (startsWith(html, position, "@endforeach")) {
            flushText();

            tokens.push_back({
                .type = TokenType::EndForeach,
                .value = "",
                .position = position
            });

            position += 11;
            textStart = position;

            continue;
        }

        if (isComponentTagStart(html, position)) {
            const auto end =
                findComponentTagEnd(html, position);

            if (end == std::string_view::npos) {
                ++position;
                continue;
            }

            flushText();

            tokens.push_back({
                .type = TokenType::ComponentTag,
                .value = std::string(
                    html.substr(position, end - position + 1)
                ),
                .position = position
            });

            position = end + 1;
            textStart = position;

            continue;
        }

        ++position;
    }

    flushText();

    return tokens;
}

} // namespace drogular::template_compiler