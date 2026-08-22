#include <drogular/template_runtime.hpp>
#include <drogular/template_expression.hpp>
#include <drogular/render_context.hpp>

#include <json/json.h>

#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace drogular::template_compiler {

namespace {

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

std::string escapeHtml(std::string_view value) {
    std::string output;

    for (const auto ch : value) {
        switch (ch) {
            case '&': output += "&amp;"; break;
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '"': output += "&quot;"; break;
            case '\'': output += "&#39;"; break;
            default: output += ch; break;
        }
    }

    return output;
}

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
    if (value.isString()) return value.asString();
    if (value.isInt()) return std::to_string(value.asInt());
    if (value.isUInt()) return std::to_string(value.asUInt());
    if (value.isDouble()) return std::to_string(value.asDouble());
    if (value.isBool()) return value.asBool() ? "true" : "false";
    if (value.isNull()) return "";

    return std::nullopt;
}

std::optional<std::string> resolveToString(
    std::string_view expression,
    const RenderContext& context
) {
    const auto key = trim(expression);

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

    if (const auto json = resolveJsonValue(key, context)) {
        return jsonValueToString(*json);
    }

    return std::nullopt;
}

} // namespace

std::optional<std::string> resolveVariable(
    std::string_view expression,
    const RenderContext& context
) {
    const auto value = resolveToString(expression, context);

    if (!value.has_value()) {
        return std::nullopt;
    }

    return escapeHtml(*value);
}

std::optional<std::string> resolveRawVariable(
    std::string_view expression,
    const RenderContext& context
) {
    return resolveToString(expression, context);
}

std::optional<ForeachExpressionError> validateForeachExpression(
    std::string_view expression
) {
    const auto inPosition = expression.find(" in ");

    if (inPosition == std::string_view::npos) {
        return ForeachExpressionError{
            .message = "Expected 'in'",
            .position = expression.size()
        };
    }

    const auto variable = trim(expression.substr(0, inPosition));
    if (variable.empty()) {
        return ForeachExpressionError{
            .message = "Expected loop variable before 'in'",
            .position = 0
        };
    }

    if (!(std::isalpha(static_cast<unsigned char>(variable.front())) ||
        variable.front() == '_')
    ) {
        return ForeachExpressionError{
            .message = "Invalid loop variable '" + variable + "'",
            .position = 0
        };
    }

    for (std::size_t index = 1; index < variable.size(); ++index) {
        const auto ch = static_cast<unsigned char>(variable[index]);
        if (!std::isalnum(ch) && variable[index] != '_') {
            return ForeachExpressionError{
                .message = "Invalid loop variable '" + variable + "'",
                .position = index
            };
        }
    }

    const auto remainderStart = inPosition + 4;
    const auto remainder = expression.substr(remainderStart);
    const auto wherePosition = remainder.find(" where ");
    const auto collectionPart = wherePosition == std::string_view::npos
        ? remainder
        : remainder.substr(0, wherePosition);
    const auto collection = trim(collectionPart);

    if (collection.empty()) {
        return ForeachExpressionError{
            .message = "Expected collection after 'in'",
            .position = remainderStart
        };
    }

    if (wherePosition != std::string_view::npos) {
        const auto conditionStart = remainderStart + wherePosition + 7;
        const auto condition = trim(expression.substr(conditionStart));

        if (condition.empty()) {
            return ForeachExpressionError{
                .message = "Expected condition after 'where'",
                .position = conditionStart
            };
        }
    }

    return std::nullopt;
}

std::optional<ForeachExpression> parseForeachExpression(
    std::string_view expression
) {
    if (validateForeachExpression(expression).has_value()) {
        return std::nullopt;
    }

    const auto inPosition = expression.find(" in ");
    const auto remainderStart = inPosition + 4;
    const auto remainder = expression.substr(remainderStart);
    const auto wherePosition = remainder.find(" where ");

    ForeachExpression result{
        .variable = trim(expression.substr(0, inPosition)),
        .collection = trim(
            wherePosition == std::string_view::npos
                ? remainder
                : remainder.substr(0, wherePosition)
        )
    };

    if (wherePosition != std::string_view::npos) {
        const auto rawConditionStart = remainderStart + wherePosition + 7;
        auto conditionStart = rawConditionStart;
        while (conditionStart < expression.size() &&
            std::isspace(static_cast<unsigned char>(expression[conditionStart]))
        ) {
            ++conditionStart;
        }

        result.condition = trim(expression.substr(rawConditionStart));
        result.conditionPosition = conditionStart;
    }

    return result;
}

std::optional<ConditionExpressionError> validateConditionExpression(
    std::string_view expression
) {
    return template_expression::parse(expression).error;
}

bool evaluateCondition(
    std::string_view expression,
    const RenderContext& context
) {
    const auto result = template_expression::parse(expression);
    if (!result) {
        return false;
    }

    return template_expression::evaluate(*result.expression, context).truthy();
}

std::optional<Json::Value> resolveJsonValue(
    std::string_view expression,
    const RenderContext& context
) {
    const auto key = trim(expression);

    const auto parts = splitPath(key);

    if (parts.empty()) {
        return std::nullopt;
    }

    const auto root = context.get<Json::Value>(parts[0]);

    if (!root.has_value()) {
        return std::nullopt;
    }

    if (parts.size() == 1) {
        return *root;
    }

    Json::Value current = *root;

    for (size_t i = 1; i < parts.size(); ++i) {
        if (!current.isObject() || !current.isMember(parts[i])) {
            return std::nullopt;
        }

        current = current[parts[i]];
    }

    return current;
}

namespace detail {

void setLoopMetadata(
    RenderContext& childContext,
    const RenderContext& parentContext,
    std::size_t index,
    std::size_t count
) {
    constexpr std::string_view internalLoopKey =
        "__drogular_template_loop_metadata";

    Json::Value loop(Json::objectValue);
    loop["index"] = static_cast<Json::UInt64>(index);
    loop["number"] = static_cast<Json::UInt64>(index + 1);
    loop["first"] = index == 0;
    loop["last"] = index + 1 == count;
    loop["count"] = static_cast<Json::UInt64>(count);

    if (const auto parent =
        parentContext.get<Json::Value>(std::string(internalLoopKey))) {
        loop["parent"] = *parent;
        const auto parentDepth = (*parent)["depth"].asUInt64();
        loop["depth"] = static_cast<Json::UInt64>(parentDepth + 1);
    } else {
        loop["parent"] = Json::Value(Json::nullValue);
        loop["depth"] = static_cast<Json::UInt64>(0);
    }

    childContext.set("loop", loop);
    childContext.set(std::string(internalLoopKey), std::move(loop));
}

} // namespace detail

} // namespace drogular::template_compiler