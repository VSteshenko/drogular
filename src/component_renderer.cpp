#include <drogular/component_renderer.hpp>
#include <drogular/testing.hpp>

#include <string>
#include <unordered_map>

namespace drogular::component_renderer {

namespace {

bool isTagNameStart(char ch) {
    return std::isupper(static_cast<unsigned char>(ch));
}

bool isTagNameChar(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) ||
           ch == '_' ||
           ch == '-';
}

std::unordered_map<std::string, std::string> parseAttributes(std::string_view value) {
    std::unordered_map<std::string, std::string> attributes;
    size_t position = 0;

    while (position < value.size()) {
        while (position < value.size() &&
               std::isspace(static_cast<unsigned char>(value[position]))) {
            ++position;
        }

        size_t keyStart = position;

        while (position < value.size() &&
               isTagNameChar(value[position])) {
            ++position;
        }

        if (keyStart == position) {
            break;
        }

        const auto key =
            std::string(value.substr(keyStart, position - keyStart));

        while (position < value.size() &&
               std::isspace(static_cast<unsigned char>(value[position]))) {
            ++position;
        }

        if (position >= value.size() || value[position] != '=') {
            break;
        }

        ++position;

        while (position < value.size() &&
               std::isspace(static_cast<unsigned char>(value[position]))) {
            ++position;
        }

        if (position >= value.size() || value[position] != '"') {
            break;
        }

        ++position;

        const auto valueStart = position;

        while (position < value.size() && value[position] != '"') {
            ++position;
        }

        if (position >= value.size()) {
            break;
        }

        attributes[key] = std::string(value.substr(valueStart, position - valueStart));

        ++position;
    }

    return attributes;
}

} // namespace

std::string render(
    std::string_view html,
    const ComponentRegistry& registry,
    RenderContext& context
) {
    std::string output;
    size_t position = 0;

    while (position < html.size()) {
        const auto tagStart = html.find('<', position);

        if (tagStart == std::string_view::npos) {
            output.append(html.substr(position));
            break;
        }

        output.append(html.substr(position, tagStart - position));

        const auto nameStart = tagStart + 1;

        if (nameStart >= html.size() ||
            !isTagNameStart(html[nameStart])) {
            output += html[tagStart];
            position = tagStart + 1;
            continue;
        }

        size_t nameEnd = nameStart;

        while (nameEnd < html.size() &&
               isTagNameChar(html[nameEnd])) {
            ++nameEnd;
        }

        const auto tagName =
            std::string(html.substr(nameStart, nameEnd - nameStart));

        size_t cursor = nameEnd;

        const auto attributesStart = cursor;

        while (cursor < html.size()) {
            if (cursor + 1 < html.size() &&
                html[cursor] == '/' &&
                html[cursor + 1] == '>') {
                break;
            }

            ++cursor;
        }

        if (cursor + 1 >= html.size() ||
            html[cursor] != '/' ||
            html[cursor + 1] != '>') {
            output.append(html.substr(tagStart, nameEnd - tagStart));
            position = nameEnd;
            continue;
        }

        const auto attributesText =
            html.substr(attributesStart, cursor - attributesStart);

        const auto tagEnd = cursor + 2;

        const auto component = registry.create(tagName);

        if (component == nullptr) {
            output.append(html.substr(tagStart, tagEnd - tagStart));
        } else {
            const auto attributes = parseAttributes(attributesText);

            for (const auto& [name, value] : attributes) {
                component->setInput(name, value);
            }

            auto childContext = context.createChild();

            output += drogular::test::renderComponentTree(
                *component,
                childContext
            );
        }

        position = tagEnd;
    }

    return output;
}

} // namespace drogular::component_renderer
