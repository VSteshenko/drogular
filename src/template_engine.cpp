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

/**
 * Escapes text for safe HTML output.
 */
std::string escapeHtml(std::string_view value) {
    std::string output;

    for (const auto ch : value) {
        switch (ch) {
        case '&':
            output += "&amp;";
            break;
        case '<':
            output += "&lt;";
            break;
        case '>':
            output += "&gt;";
            break;
        case '"':
            output += "&quot;";
            break;
        case '\'':
            output += "&#39;";
            break;
        default:
            output += ch;
            break;
        }
    }

    return output;
}

/**
 * Renders simple @if(condition) ... @endif blocks.
 */
std::string renderIfBlocks(
    std::string_view html,
    const RenderContext& context
) {
    std::string output;
    size_t position = 0;

    while (position < html.size()) {
        const auto ifStart = html.find("@if(", position);

        if (ifStart == std::string_view::npos) {
            output.append(html.substr(position));
            break;
        }

        output.append(html.substr(position, ifStart - position));

        const auto conditionEnd = html.find(")", ifStart + 4);

        if (conditionEnd == std::string_view::npos) {
            output.append(html.substr(ifStart));
            break;
        }

        const auto blockEnd = html.find("@endif", conditionEnd + 1);

        if (blockEnd == std::string_view::npos) {
            output.append(html.substr(ifStart));
            break;
        }

        const auto conditionName = trim(
            html.substr(ifStart + 4, conditionEnd - ifStart - 4)
        );

        const auto blockStart = conditionEnd + 1;
        const auto elseStart = html.find("@else", blockStart);

        const bool hasElse =
            elseStart != std::string_view::npos &&
            elseStart < blockEnd;

        const auto trueBlock = hasElse
            ? html.substr(blockStart, elseStart - blockStart)
            : html.substr(blockStart, blockEnd - blockStart);

        const auto falseBlock = hasElse
            ? html.substr(elseStart + std::string_view("@else").size(),
                          blockEnd - elseStart - std::string_view("@else").size())
            : std::string_view{};

        const auto condition =
            context.get<bool>(conditionName).value_or(false);

        if (condition) {
            output.append(trueBlock);
        } else {
            output.append(falseBlock);
        }

        position = blockEnd + std::string_view("@endif").size();
    }

    return output;
}

} // namespace

std::string render(
    std::string_view html,
    const RenderContext& context
) {
    const auto processedIfBlocks = renderIfBlocks(html, context);
    html = processedIfBlocks;

    std::string output;
    size_t position = 0;

    while (position < html.size()) {
        const auto rawStart = html.find("{{{", position);
        const auto escapedStart = html.find("{{", position);

        if (rawStart == std::string_view::npos &&
            escapedStart == std::string_view::npos) {
            output.append(html.substr(position));
            break;
            }

        const bool useRaw =
            rawStart != std::string_view::npos &&
            rawStart == escapedStart;

        const auto start = useRaw ? rawStart : escapedStart;

        output.append(html.substr(position, start - position));

        if (useRaw) {
            const auto end = html.find("}}}", start + 3);

            if (end == std::string_view::npos) {
                output.append(html.substr(start));
                break;
            }

            const auto key = trim(
                html.substr(start + 3, end - start - 3)
            );

            const auto value = valueToString(context, key);

            if (value.has_value()) {
                output += *value;
            }

            position = end + 3;
        } else {
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
                output += escapeHtml(*value);
            }

            position = end + 2;
        }
    }

    return output;
}

} // namespace drogular::template_engine
