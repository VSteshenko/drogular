#include <drogular/template_engine.hpp>

#include <optional>
#include <string>

namespace drogular::template_engine {

namespace {

/**
 * Trims whitespace from both sides of a string.
 */
std::string trim(std::string_view value) {
    size_t start = 0;
    size_t end = value.size();

    while (start < end &&
           std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    while (end > start &&
           std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return std::string(value.substr(start, end - start));
}

/**
 * Converts a render context value to text.
 */
std::optional<std::string> valueToString(
    const RenderContext& context,
    const std::string& key
) {
    if (const auto value = context.get<std::string>(key)) {
        return *value;
    }

    if (const auto value = context.get<int>(key)) {
        return std::to_string(*value);
    }

    if (const auto value = context.get<double>(key)) {
        return std::to_string(*value);
    }

    if (const auto value = context.get<bool>(key)) {
        return *value ? "true" : "false";
    }

    return std::nullopt;
}

} // namespace

std::string render(
    std::string_view html,
    const RenderContext& context
) {
    std::string output;
    size_t position = 0;

    while (position < html.size()) {
        const auto start = html.find("{{", position);

        if (start == std::string_view::npos) {
            output.append(html.substr(position));
            break;
        }

        output.append(html.substr(position, start - position));

        const auto end = html.find("}}", start + 2);

        if (end == std::string_view::npos) {
            output.append(html.substr(start));
            break;
        }

        const auto key = trim(
            html.substr(start + 2, end - start - 2)
        );

        const auto value = valueToString(context, key);

        if (value.has_value()) {
            output += *value;
        }
        position = end + 2;
    }

    return output;
}

} // namespace drogular::template_engine
