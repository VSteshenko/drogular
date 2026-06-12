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

} // namespace

std::vector<Token> tokenize(std::string_view html) {
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
                html.find(")", position + 4);

            if (end == std::string_view::npos) {
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

        if (startsWith(html, position, "@foreach(")) {
            flushText();

            const auto end =
                html.find(")", position + 9);

            if (end == std::string_view::npos) {
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
