#include <drogular/component_renderer.hpp>
#include <drogular/testing.hpp>

#include <cctype>

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

        while (cursor < html.size() &&
               std::isspace(static_cast<unsigned char>(html[cursor]))) {
            ++cursor;
        }

        if (cursor + 1 >= html.size() ||
            html[cursor] != '/' ||
            html[cursor + 1] != '>') {
            output.append(html.substr(tagStart, nameEnd - tagStart));
            position = nameEnd;
            continue;
        }

        const auto tagEnd = cursor + 2;

        const auto component = registry.create(tagName);

        if (component == nullptr) {
            output.append(html.substr(tagStart, tagEnd - tagStart));
        } else {
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
