#include <drogular/template_runtime.hpp>
#include <drogular/template/expression/expression.hpp>
#include <drogular/render_context.hpp>

#include <json/json.h>

#include <optional>
#include <cmath>
#include <limits>
#include <iomanip>
#include <variant>
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

std::size_t findTopLevelWhere(
    std::string_view expression,
    std::size_t from
) {
    std::size_t parenDepth = 0;
    std::size_t bracketDepth = 0;
    char quote = '\0';
    bool escaped = false;

    for (std::size_t i = from; i < expression.size(); ++i) {
        const char ch = expression[i];
        if (quote != '\0') {
            if (escaped) {
                escaped = false;
                continue;
            }
            if (ch == '\\') {
                escaped = true;
                continue;
            }
            if (ch == quote) {
                quote = '\0';
            }
            continue;
        }

        if (ch == '\'' || ch == '"') {
            quote = ch;
            continue;
        }
        if (ch == '(') {
            ++parenDepth;
            continue;
        }
        if (ch == ')') {
            if (parenDepth > 0) --parenDepth;
            continue;
        }
        if (ch == '[') {
            ++bracketDepth;
            continue;
        }
        if (ch == ']') {
            if (bracketDepth > 0) --bracketDepth;
            continue;
        }

        if (parenDepth != 0 || bracketDepth != 0 || ch != 'w') {
            continue;
        }
        constexpr std::string_view keyword = "where";
        if (i + keyword.size() > expression.size() ||
            expression.substr(i, keyword.size()) != keyword) {
            continue;
        }
        if (i == from ||
            !std::isspace(static_cast<unsigned char>(expression[i - 1]))) {
            continue;
        }
        const auto after = i + keyword.size();
        if (after >= expression.size() ||
            !std::isspace(static_cast<unsigned char>(expression[after]))) {
            continue;
        }
        return i;
    }

    return std::string_view::npos;
}

std::optional<std::string> expressionValueToString(
    const template_expression::ExpressionValue& value
) {
    if (const auto string = value.string()) {
        return *string;
    }
    if (const auto boolean = value.boolean()) {
        return *boolean ? "true" : "false";
    }
    if (const auto number = value.number()) {
        std::ostringstream stream;
        stream << std::setprecision(15) << *number;
        return stream.str();
    }
    if (const auto* json = std::get_if<Json::Value>(&value.storage())) {
        return jsonValueToString(*json);
    }
    if (value.isNull()) {
        return std::string{};
    }

    return std::nullopt;
}

} // namespace

std::optional<std::string> resolveVariable(
    std::string_view expression,
    const RenderContext& context
) {
    const template_expression::BindingContext bindings(context);
    return resolveVariable(expression, bindings);
}

std::optional<std::string> resolveVariable(
    std::string_view expression,
    const template_expression::BindingContext& context
) {
    const auto key = trim(expression);
    std::optional<std::string> value;

    // Preserve the historical RenderContext formatting contract for direct
    // values when no lexical binding shadows the root name. This keeps the
    // compatibility entry point stable while all rendering goes through the
    // compiled runtime.
    const auto rootEnd = key.find('.');
    const auto rootName = key.substr(0, rootEnd);
    if (!context.contains(rootName)) {
        value = resolveToString(key, context.renderContext());
    }

    if (!value.has_value()) {
        const auto resolved = context.resolve(key);
        if (!resolved.isNull()) {
            value = expressionValueToString(resolved);
        }
    }

    if (!value.has_value()) {
        const auto parsed = template_expression::parse(expression);
        if (!parsed) {
            return std::nullopt;
        }
        value = expressionValueToString(
            template_expression::evaluate(*parsed.expression, context)
        );
    }

    if (!value.has_value()) {
        return std::nullopt;
    }

    return escapeHtml(*value);
}

std::optional<std::string> resolveRawVariable(
    std::string_view expression,
    const RenderContext& context
) {
    const template_expression::BindingContext bindings(context);
    return resolveRawVariable(expression, bindings);
}

std::optional<std::string> resolveRawVariable(
    std::string_view expression,
    const template_expression::BindingContext& context
) {
    const auto key = trim(expression);
    const auto rootEnd = key.find('.');
    const auto rootName = key.substr(0, rootEnd);
    if (!context.contains(rootName)) {
        if (const auto legacy = resolveToString(key, context.renderContext())) {
            return legacy;
        }
    }

    const auto direct = context.resolve(key);
    if (!direct.isNull()) {
        if (const auto resolved = expressionValueToString(direct)) {
            return resolved;
        }
    }

    const auto parsed = template_expression::parse(expression);
    if (!parsed) {
        return std::nullopt;
    }

    return expressionValueToString(
        template_expression::evaluate(*parsed.expression, context)
    );
}

std::optional<BindingExpression> parseBindingExpression(
    std::string_view expression
) {
    std::size_t position = 0;
    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))
    ) {
        ++position;
    }

    if (position >= expression.size() ||
        !(std::isalpha(static_cast<unsigned char>(expression[position])) ||
          expression[position] == '_')
    ) {
        return std::nullopt;
    }

    const auto nameStart = position++;
    while (position < expression.size() &&
           (std::isalnum(static_cast<unsigned char>(expression[position])) ||
            expression[position] == '_')
    ) {
        ++position;
    }
    const auto name = std::string(
        expression.substr(nameStart, position - nameStart)
    );

    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))
    ) {
        ++position;
    }
    if (position >= expression.size() || expression[position] != '=' ||
        (position + 1 < expression.size() && expression[position + 1] == '=')
    ) {
        return std::nullopt;
    }
    ++position;
    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))
    ) {
        ++position;
    }
    if (position >= expression.size()) {
        return std::nullopt;
    }

    const auto rhs = trim(expression.substr(position));
    const auto parsed = template_expression::parse(rhs);
    if (!parsed) {
        return std::nullopt;
    }

    return BindingExpression{
        .name = name,
        .expression = rhs,
        .compiledExpression = parsed.expression,
        .expressionPosition = position
    };
}

std::optional<BindingExpressionError> validateBindingExpression(
    std::string_view expression
) {
    std::size_t position = 0;
    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))) {
        ++position;
    }
    if (position >= expression.size() ||
        !(std::isalpha(static_cast<unsigned char>(expression[position])) ||
          expression[position] == '_')) {
        return BindingExpressionError{
            .message = "Expected binding identifier",
            .position = position
        };
    }
    ++position;
    while (position < expression.size() &&
           (std::isalnum(static_cast<unsigned char>(expression[position])) ||
            expression[position] == '_')) {
        ++position;
    }
    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))) {
        ++position;
    }
    if (position >= expression.size() || expression[position] != '=' ||
        (position + 1 < expression.size() && expression[position + 1] == '=')) {
        return BindingExpressionError{
            .message = "Expected '=' after binding identifier",
            .position = position
        };
    }
    ++position;
    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))) {
        ++position;
    }
    if (position >= expression.size()) {
        return BindingExpressionError{
            .message = "Expected expression after '='",
            .position = position
        };
    }

    const auto rhs = expression.substr(position);
    const auto parsed = template_expression::parse(rhs);
    if (!parsed) {
        return BindingExpressionError{
            .message = parsed.error->message,
            .position = position + parsed.error->position
        };
    }

    return std::nullopt;
}

std::optional<LetExpression> parseLetExpression(
    std::string_view expression
) {
    return parseBindingExpression(expression);
}

std::optional<LetExpressionError> validateLetExpression(
    std::string_view expression
) {
    return validateBindingExpression(expression);
}

std::optional<ForeachExpressionError> validateForeachExpression(
    std::string_view expression
) {
    const auto parsed = parseForeachExpression(expression);
    if (parsed.has_value()) {
        return std::nullopt;
    }

    std::size_t position = 0;
    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))
    ) {
        ++position;
    }

    if (position >= expression.size() ||
        !(std::isalpha(static_cast<unsigned char>(expression[position])) ||
          expression[position] == '_')
    ) {
        return ForeachExpressionError{
            .message = "Expected loop variable before 'in'",
            .position = position
        };
    }

    const auto variableStart = position;
    ++position;
    while (position < expression.size() &&
           (std::isalnum(static_cast<unsigned char>(expression[position])) ||
            expression[position] == '_')
    ) {
        ++position;
    }

    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))
    ) {
        ++position;
    }

    if (position + 2 > expression.size() ||
        expression.substr(position, 2) != "in" ||
        (position + 2 < expression.size() &&
         !std::isspace(static_cast<unsigned char>(expression[position + 2])))
    ) {
        return ForeachExpressionError{
            .message = "Expected 'in'",
            .position = position
        };
    }

    position += 2;
    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))
    ) {
        ++position;
    }
    if (position >= expression.size()) {
        return ForeachExpressionError{
            .message = "Expected collection after 'in'",
            .position = position
        };
    }

    // Re-scan the top-level `where` separator to report expression errors
    // at their precise position.
    const auto wherePosition = findTopLevelWhere(expression, position);
    auto collectionEnd = wherePosition == std::string_view::npos
        ? expression.size()
        : wherePosition;
    while (collectionEnd > position &&
           std::isspace(static_cast<unsigned char>(expression[collectionEnd - 1]))
    ) {
        --collectionEnd;
    }
    if (collectionEnd == position) {
        return ForeachExpressionError{
            .message = "Expected collection after 'in'",
            .position = position
        };
    }

    const auto collection =
        expression.substr(position, collectionEnd - position);
    if (const auto result = template_expression::parse(collection); !result) {
        return ForeachExpressionError{
            .message = "Invalid collection expression: " + result.error->message,
            .position = position + result.error->position
        };
    }

    if (wherePosition != std::string_view::npos) {
        auto conditionStart = wherePosition + 5;
        while (conditionStart < expression.size() &&
               std::isspace(static_cast<unsigned char>(expression[conditionStart]))) {
            ++conditionStart;
        }
        if (conditionStart >= expression.size()) {
            return ForeachExpressionError{
                .message = "Expected condition after 'where'",
                .position = conditionStart
            };
        }
    }

    return ForeachExpressionError{
        .message = "Invalid @foreach expression",
        .position = variableStart
    };
}

std::optional<ForeachExpression> parseForeachExpression(
    std::string_view expression
) {
    std::size_t position = 0;
    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))
    ) {
        ++position;
    }
    if (position >= expression.size() ||
        !(std::isalpha(static_cast<unsigned char>(expression[position])) ||
          expression[position] == '_')
    ) {
        return std::nullopt;
    }

    const auto variableStart = position;
    ++position;
    while (position < expression.size() &&
           (std::isalnum(static_cast<unsigned char>(expression[position])) ||
            expression[position] == '_')
    ) {
        ++position;
    }
    const auto variable =
        std::string(expression.substr(variableStart, position - variableStart));

    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))
    ) {
        ++position;
    }
    if (position + 2 > expression.size() || expression.substr(position, 2) != "in" ||
        (position + 2 < expression.size() &&
         !std::isspace(static_cast<unsigned char>(expression[position + 2])))
    ) {
        return std::nullopt;
    }
    position += 2;
    while (position < expression.size() &&
           std::isspace(static_cast<unsigned char>(expression[position]))
    ) {
        ++position;
    }
    const auto collectionStart = position;
    if (collectionStart >= expression.size()) {
        return std::nullopt;
    }

    const auto wherePosition = findTopLevelWhere(expression, collectionStart);

    auto collectionEnd = wherePosition == std::string_view::npos
        ? expression.size()
        : wherePosition;
    while (collectionEnd > collectionStart &&
           std::isspace(static_cast<unsigned char>(expression[collectionEnd - 1]))
    ) {
        --collectionEnd;
    }
    if (collectionEnd == collectionStart) {
        return std::nullopt;
    }

    const auto collection = std::string(
        expression.substr(collectionStart, collectionEnd - collectionStart));
    const auto parsedCollection =
        template_expression::parse(collection);
    if (!parsedCollection) {
        return std::nullopt;
    }

    ForeachExpression result{
        .variable = variable,
        .collection = collection,
        .collectionExpression = parsedCollection.expression,
        .collectionPosition = collectionStart,
        .condition = std::nullopt,
        .conditionExpression = nullptr,
        .conditionPosition = 0
    };

    if (wherePosition != std::string_view::npos) {
        auto conditionStart = wherePosition + 5;
        while (conditionStart < expression.size() &&
               std::isspace(static_cast<unsigned char>(expression[conditionStart]))
        ) {
            ++conditionStart;
        }
        if (conditionStart >= expression.size()) {
            return std::nullopt;
        }
        result.condition = trim(expression.substr(conditionStart));
        result.conditionPosition = conditionStart;
        const auto parsedCondition =
            template_expression::parse(*result.condition);
        result.conditionExpression = parsedCondition.expression;
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
    const template_expression::BindingContext bindings(context);
    return evaluateCondition(expression, bindings);
}

bool evaluateCondition(
    std::string_view expression,
    const template_expression::BindingContext& context
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

void setExpressionValue(
    RenderContext& context,
    std::string key,
    template_expression::ExpressionValue value
) {
    const auto& storage = value.storage();
    if (const auto* boolean = std::get_if<bool>(&storage)) {
        context.set(std::move(key), *boolean);
        return;
    }
    if (const auto* number = std::get_if<double>(&storage)) {
        if (std::isfinite(*number) && std::trunc(*number) == *number &&
            *number >= static_cast<double>(std::numeric_limits<int>::min()) &&
            *number <= static_cast<double>(std::numeric_limits<int>::max())
        ) {
            context.set(std::move(key), static_cast<int>(*number));
        } else {
            context.set(std::move(key), *number);
        }
        return;
    }
    if (const auto* string = std::get_if<std::string>(&storage)) {
        context.set(std::move(key), *string);
        return;
    }
    if (const auto* json = std::get_if<Json::Value>(&storage)) {
        context.set(std::move(key), *json);
        return;
    }
    if (std::holds_alternative<std::monostate>(storage)) {
        context.set(std::move(key), Json::Value(Json::nullValue));
        return;
    }

    // Native List/Range values have no RenderContext counterpart. Preserve
    // the ExpressionValue so nested expressions and future bindings can use
    // them without lossy conversion.
    context.set(std::move(key), std::move(value));
}

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