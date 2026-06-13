#include <drogular/template_runtime.hpp>

#include <json/json.h>

#include <cctype>
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

bool evaluateCondition(
    std::string_view expression,
    const RenderContext& context
) {
    const auto key = trim(expression);

    if (const auto value = context.get<bool>(key)) {
        return *value;
    }

    if (const auto json = resolveJsonValue(key, context)) {
        if (json->isBool()) return json->asBool();
        if (json->isString()) return !json->asString().empty();
        if (json->isInt()) return json->asInt() != 0;
        if (json->isUInt()) return json->asUInt() != 0;
        if (json->isDouble()) return json->asDouble() != 0.0;
        if (json->isArray() || json->isObject()) return !json->empty();
    }

    if (const auto value = resolveToString(key, context)) {
        return !value->empty() &&
               *value != "false" &&
               *value != "0";
    }

    return false;
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

} // namespace drogular::template_compiler
