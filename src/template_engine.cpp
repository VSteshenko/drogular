#include <drogular/template_engine.hpp>
#include <drogular/render_context.hpp>
#include <drogular/template_runtime.hpp>

#include <optional>
#include <string>
#include <json/json.h>
#include <sstream>
#include <vector>

namespace drogular::template_engine {

namespace {

std::vector<std::string> splitPath(const std::string& key) {
    std::vector<std::string> parts;
    std::stringstream stream(key);
    std::string part;

    while (std::getline(stream, part, '.')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }

    return parts;
}

std::optional<std::string> jsonValueToString(const Json::Value& value) {
    if (value.isString()) {
        return value.asString();
    }

    if (value.isInt()) {
        return std::to_string(value.asInt());
    }

    if (value.isUInt()) {
        return std::to_string(value.asUInt());
    }

    if (value.isDouble()) {
        return std::to_string(value.asDouble());
    }

    if (value.isBool()) {
        return value.asBool() ? "true" : "false";
    }

    if (value.isNull()) {
        return "";
    }

    return std::nullopt;
}

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

struct ResolvedValue {
    enum class Type {
        Missing,
        String,
        Json
    };

    Type type = Type::Missing;
    std::string stringValue;
    Json::Value jsonValue;
};

ResolvedValue resolveValue(
    const RenderContext& context,
    const std::string& key
) {
    const auto parts = splitPath(key);

    if (parts.empty()) {
        return {};
    }

    if (parts.size() == 1) {
        if (const auto value = context.get<std::string>(key)) {
            return {
                .type = ResolvedValue::Type::String,
                .stringValue = *value
            };
        }

        if (const auto value = context.get<int>(key)) {
            return {
                .type = ResolvedValue::Type::String,
                .stringValue = std::to_string(*value)
            };
        }

        if (const auto value = context.get<double>(key)) {
            return {
                .type = ResolvedValue::Type::String,
                .stringValue = std::to_string(*value)
            };
        }

        if (const auto value = context.get<bool>(key)) {
            return {
                .type = ResolvedValue::Type::String,
                .stringValue = *value ? "true" : "false"
            };
        }

        if (const auto value = context.get<Json::Value>(key)) {
            return {
                .type = ResolvedValue::Type::Json,
                .jsonValue = *value
            };
        }

        return {};
    }

    const auto root = context.get<Json::Value>(parts[0]);

    if (!root.has_value()) {
        return {};
    }

    Json::Value current = *root;

    for (size_t i = 1; i < parts.size(); ++i) {
        if (!current.isObject() || !current.isMember(parts[i])) {
            return {};
        }

        current = current[parts[i]];
    }

    return {
        .type = ResolvedValue::Type::Json,
        .jsonValue = current
    };
}

/**
 * Converts a render context value to text.
 */
std::optional<std::string> valueToString(
    const RenderContext& context,
    const std::string& key
) {
    const auto resolved = resolveValue(context, key);

    if (resolved.type == ResolvedValue::Type::String) {
        return resolved.stringValue;
    }

    if (resolved.type == ResolvedValue::Type::Json) {
        return jsonValueToString(resolved.jsonValue);
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

size_t findConditionEnd(
    std::string_view html,
    size_t position
);

enum class LoopControl {
    None,
    Break,
    Continue
};

struct ForeachBlockBounds {
    size_t blockEnd = std::string_view::npos;
    size_t emptyStart = std::string_view::npos;
};

ForeachBlockBounds findForeachBlockBounds(
    std::string_view html,
    size_t blockStart
) {
    size_t depth = 1;
    size_t position = blockStart;
    size_t emptyStart = std::string_view::npos;

    while (position < html.size()) {
        const auto nestedStart = html.find("@foreach(", position);
        const auto empty = html.find("@empty", position);
        const auto end = html.find("@endforeach", position);

        auto next = end;
        enum class Match { End, Foreach, Empty } match = Match::End;

        if (nestedStart != std::string_view::npos &&
            (next == std::string_view::npos || nestedStart < next)) {
            next = nestedStart;
            match = Match::Foreach;
        }

        if (empty != std::string_view::npos &&
            (next == std::string_view::npos || empty < next)) {
            next = empty;
            match = Match::Empty;
        }

        if (next == std::string_view::npos) {
            return {};
        }

        if (match == Match::Foreach) {
            ++depth;
            position = next + 9;
            continue;
        }

        if (match == Match::Empty) {
            if (depth == 1 && emptyStart == std::string_view::npos) {
                emptyStart = next;
            }
            position = next + 6;
            continue;
        }

        --depth;
        if (depth == 0) {
            return {
                .blockEnd = next,
                .emptyStart = emptyStart
            };
        }

        position = next + std::string_view("@endforeach").size();
    }

    return {};
}

LoopControl appendRenderedIteration(
    std::string rendered,
    std::string& output
) {
    const auto breakPosition = rendered.find("@break");
    const auto continuePosition = rendered.find("@continue");

    if (breakPosition == std::string::npos &&
        continuePosition == std::string::npos) {
        output += rendered;
        return LoopControl::None;
    }

    if (breakPosition != std::string::npos &&
        (continuePosition == std::string::npos || breakPosition < continuePosition)) {
        output.append(rendered, 0, breakPosition);
        return LoopControl::Break;
    }

    output.append(rendered, 0, continuePosition);
    return LoopControl::Continue;
}

std::string renderForeachBlocks(
    std::string_view html,
    const RenderContext& context
) {
    std::string output;
    size_t position = 0;

    while (position < html.size()) {
        const auto foreachStart = html.find("@foreach(", position);

        if (foreachStart == std::string_view::npos) {
            output.append(html.substr(position));
            break;
        }

        output.append(html.substr(position, foreachStart - position));

        const auto headerEnd = findConditionEnd(html, foreachStart + 9);

        if (headerEnd == std::string_view::npos) {
            output.append(html.substr(foreachStart));
            break;
        }

        const auto bounds = findForeachBlockBounds(html, headerEnd + 1);
        if (bounds.blockEnd == std::string_view::npos) {
            output.append(html.substr(foreachStart));
            break;
        }

        const auto rawExpression = html.substr(
            foreachStart + 9,
            headerEnd - foreachStart - 9
        );
        const auto expression =
            template_compiler::parseForeachExpression(rawExpression);

        if (!expression.has_value()) {
            output.append(html.substr(foreachStart));
            break;
        }

        const auto bodyEnd = bounds.emptyStart == std::string_view::npos
            ? bounds.blockEnd
            : bounds.emptyStart;

        const auto templateBlock = std::string(
            html.substr(headerEnd + 1, bodyEnd - headerEnd - 1)
        );

        const auto emptyBlock = bounds.emptyStart == std::string_view::npos
            ? std::string{}
            : std::string(html.substr(
                bounds.emptyStart + std::string_view("@empty").size(),
                bounds.blockEnd - bounds.emptyStart - std::string_view("@empty").size()
            ));

        const auto makeLoop = [](std::size_t index, std::size_t count) {
            Json::Value loop(Json::objectValue);
            loop["index"] = static_cast<Json::UInt64>(index);
            loop["number"] = static_cast<Json::UInt64>(index + 1);
            loop["first"] = index == 0;
            loop["last"] = index + 1 == count;
            loop["count"] = static_cast<Json::UInt64>(count);
            return loop;
        };

        std::size_t renderedCount = 0;

        if (const auto stringValues =
            context.get<std::vector<std::string>>(expression->collection)) {
            std::vector<std::size_t> selected;
            selected.reserve(stringValues->size());

            for (std::size_t index = 0; index < stringValues->size(); ++index) {
                if (expression->condition.has_value()) {
                    auto conditionContext = context.createChild();
                    conditionContext.set(expression->variable, (*stringValues)[index]);
                    if (!template_compiler::evaluateCondition(
                            *expression->condition,
                            conditionContext)) {
                        continue;
                    }
                }
                selected.push_back(index);
            }

            renderedCount = selected.size();
            for (std::size_t index = 0; index < selected.size(); ++index) {
                auto itemContext = context.createChild();
                itemContext.set(
                    expression->variable,
                    (*stringValues)[selected[index]]
                );
                itemContext.set("loop", makeLoop(index, selected.size()));

                const auto control = appendRenderedIteration(
                    render(templateBlock, itemContext),
                    output
                );
                if (control == LoopControl::Break) {
                    break;
                }
            }
        } else if (const auto jsonValues =
            template_compiler::resolveJsonValue(expression->collection, context);
            jsonValues.has_value() && jsonValues->isArray()) {
            std::vector<Json::Value> selected;
            selected.reserve(jsonValues->size());

            for (const auto& value : *jsonValues) {
                if (expression->condition.has_value()) {
                    auto conditionContext = context.createChild();
                    conditionContext.set(expression->variable, value);
                    if (!template_compiler::evaluateCondition(
                            *expression->condition,
                            conditionContext)) {
                        continue;
                    }
                }
                selected.push_back(value);
            }

            renderedCount = selected.size();
            for (std::size_t index = 0; index < selected.size(); ++index) {
                auto itemContext = context.createChild();
                itemContext.set(expression->variable, selected[index]);
                itemContext.set("loop", makeLoop(index, selected.size()));

                const auto control = appendRenderedIteration(
                    render(templateBlock, itemContext),
                    output
                );
                if (control == LoopControl::Break) {
                    break;
                }
            }
        }

        if (renderedCount == 0 && !emptyBlock.empty()) {
            output += render(emptyBlock, context);
        }

        position = bounds.blockEnd + std::string_view("@endforeach").size();
    }

    return output;
}

size_t findConditionEnd(
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
        if (ch == ')' && --depth == 0) return i;
    }

    return std::string_view::npos;
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

        const auto conditionEnd = findConditionEnd(html, ifStart + 4);

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
            template_compiler::evaluateCondition(conditionName, context);

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
    auto processed =
        renderForeachBlocks(html, context);
    processed = renderIfBlocks(processed, context);
    html = processed;

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
