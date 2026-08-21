#include <drogular/template_runtime.hpp>
#include <drogular/render_context.hpp>

#include <json/json.h>

#include <cctype>
#include <cmath>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include <utility>

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

namespace {

class ConditionValue {
public:
    using Storage = std::variant<std::monostate, bool, double, std::string, Json::Value>;

    ConditionValue() = default;
    explicit ConditionValue(bool value) : value_(value) {}
    explicit ConditionValue(double value) : value_(value) {}
    explicit ConditionValue(std::string value) : value_(std::move(value)) {}
    explicit ConditionValue(Json::Value value) : value_(std::move(value)) {}

    bool truthy() const {
        if (const auto value = std::get_if<bool>(&value_)) {
            return *value;
        }
        if (const auto value = std::get_if<double>(&value_)) {
            return *value != 0.0;
        }
        if (const auto value = std::get_if<std::string>(&value_)) {
            return !value->empty() && *value != "false" && *value != "0";
        }
        if (const auto value = std::get_if<Json::Value>(&value_)) {
            if (value->isNull()) {
                return false;
            }
            if (value->isBool()) {
                return value->asBool();
            }
            if (value->isString()) {
                return !value->asString().empty();
            }
            if (value->isNumeric()) {
                return value->asDouble() != 0.0;
            }
            if (value->isArray() || value->isObject()) {
                return !value->empty();
            }
        }

        return false;
    }

    std::optional<double> number() const {
        if (const auto value = std::get_if<double>(&value_)) {
            return *value;
        }
        if (const auto value = std::get_if<Json::Value>(&value_); value && value->isNumeric()) {
            return value->asDouble();
        }

        return std::nullopt;
    }

    std::optional<std::string> string() const {
        if (const auto value = std::get_if<std::string>(&value_)) {
            return *value;
        }
        if (const auto value = std::get_if<Json::Value>(&value_); value && value->isString()) {
            return value->asString();
        }

        return std::nullopt;
    }

    std::optional<bool> boolean() const {
        if (const auto value = std::get_if<bool>(&value_)) {
            return *value;
        }
        if (const auto value = std::get_if<Json::Value>(&value_); value && value->isBool()) {
            return value->asBool();
        }

        return std::nullopt;
    }

    bool isNull() const {
        if (std::holds_alternative<std::monostate>(value_)) {
            return true;
        }
        if (const auto value = std::get_if<Json::Value>(&value_)) {
            return value->isNull();
        }

        return false;
    }

private:
    Storage value_;
};

ConditionValue resolveConditionValue(
    std::string_view identifier,
    const RenderContext& context
) {
    const auto key = trim(identifier);

    if (const auto value = context.get<bool>(key)) {
        return ConditionValue(*value);
    }
    if (const auto value = context.get<int>(key)) {
        return ConditionValue(static_cast<double>(*value));
    }
    if (const auto value = context.get<double>(key)) {
        return ConditionValue(*value);
    }
    if (const auto value = context.get<std::string>(key)) {
        return ConditionValue(*value);
    }
    if (const auto value = resolveJsonValue(key, context)) {
        return ConditionValue(*value);
    }

    return ConditionValue();
}

bool equalValues(const ConditionValue& left, const ConditionValue& right) {
    if (left.isNull() || right.isNull()) return left.isNull() && right.isNull();

    if (const auto lhs = left.number()) {
        if (const auto rhs = right.number()) {
            return *lhs == *rhs;
        }
    }
    if (const auto lhs = left.string()) {
        if (const auto rhs = right.string()) {
            return *lhs == *rhs;
        }
    }
    if (const auto lhs = left.boolean()) {
        if (const auto rhs = right.boolean()) {
            return *lhs == *rhs;
        }
    }

    return false;
}

enum class RelationalOperator { Less, LessEqual, Greater, GreaterEqual };

bool compareValues(
    const ConditionValue& left,
    const ConditionValue& right,
    RelationalOperator op
) {
    if (const auto lhs = left.number()) {
        if (const auto rhs = right.number()) {
            switch (op) {
                case RelationalOperator::Less:
                    return *lhs < *rhs;
                case RelationalOperator::LessEqual:
                    return *lhs <= *rhs;
                case RelationalOperator::Greater:
                    return *lhs > *rhs;
                case RelationalOperator::GreaterEqual:
                    return *lhs >= *rhs;
            }
        }
    }

    if (const auto lhs = left.string()) {
        if (const auto rhs = right.string()) {
            switch (op) {
                case RelationalOperator::Less:
                    return *lhs < *rhs;
                case RelationalOperator::LessEqual:
                    return *lhs <= *rhs;
                case RelationalOperator::Greater:
                    return *lhs > *rhs;
                case RelationalOperator::GreaterEqual:
                    return *lhs >= *rhs;
            }
        }
    }

    return false;
}

class ConditionParser {
public:
    ConditionParser(
        std::string_view expression,
        const RenderContext* context = nullptr
    )
        : expression_(expression), context_(context) {}

    std::optional<bool> parse() {
        skipWhitespace();
        if (position_ == expression_.size()) {
            fail("Expected condition expression", position_);
            return std::nullopt;
        }

        const auto value = parseOr();
        skipWhitespace();

        if (!value.has_value()) {
            return std::nullopt;
        }

        if (position_ != expression_.size()) {
            fail("Unexpected token", position_);
            return std::nullopt;
        }

        return value;
    }

    const std::optional<ConditionExpressionError>& error() const {
        return error_;
    }

private:
    std::optional<bool> parseOr() {
        auto left = parseAnd();
        if (!left.has_value()) {
            return std::nullopt;
        }

        while (consume("||")) {
            const auto operatorPosition = previousTokenPosition_;
            const auto right = parseAnd();
            if (!right.has_value()) {
                fail("Expected expression after '||'", operatorPosition + 2);
                return std::nullopt;
            }
            *left = *left || *right;
        }

        return left;
    }

    std::optional<bool> parseAnd() {
        auto left = parseUnary();
        if (!left.has_value()) {
            return std::nullopt;
        }

        while (consume("&&")) {
            const auto operatorPosition = previousTokenPosition_;
            const auto right = parseUnary();
            if (!right.has_value()) {
                fail("Expected expression after '&&'", operatorPosition + 2);
                return std::nullopt;
            }
            *left = *left && *right;
        }

        return left;
    }

    std::optional<bool> parseUnary() {
        if (consume("!")) {
            const auto operatorPosition = previousTokenPosition_;
            const auto value = parseUnary();
            if (!value.has_value()) {
                fail("Expected expression after '!'", operatorPosition + 1);
                return std::nullopt;
            }

            return !*value;
        }

        if (consume("(")) {
            const auto value = parseOr();
            if (!value.has_value()) {
                return std::nullopt;
            }
            if (!consume(")")) {
                fail("Expected ')'", position_);
                return std::nullopt;
            }

            return value;
        }

        return parseComparison();
    }

    std::optional<bool> parseComparison() {
        const auto left = parseValue();
        if (!left.has_value()) {
            fail("Expected value", position_);
            return std::nullopt;
        }

        const auto parseRight = [&](std::string_view op)
            -> std::optional<ConditionValue> {
            const auto operatorPosition = previousTokenPosition_;
            const auto right = parseValue();
            if (!right.has_value()) {
                fail(
                    "Expected value after '" + std::string(op) + "'",
                    operatorPosition + op.size()
                );
            }
            return right;
        };

        if (consume("==")) {
            const auto right = parseRight("==");
            return right.has_value()
                ? std::optional<bool>(equalValues(*left, *right))
                : std::nullopt;
        }
        if (consume("!=")) {
            const auto right = parseRight("!=");
            return right.has_value()
                ? std::optional<bool>(!equalValues(*left, *right))
                : std::nullopt;
        }
        if (consume("<=")) {
            const auto right = parseRight("<=");
            return right.has_value()
                ? std::optional<bool>(compareValues(*left, *right, RelationalOperator::LessEqual))
                : std::nullopt;
        }
        if (consume(">=")) {
            const auto right = parseRight(">=");
            return right.has_value()
                ? std::optional<bool>(compareValues(*left, *right, RelationalOperator::GreaterEqual))
                : std::nullopt;
        }
        if (consume("<")) {
            const auto right = parseRight("<");
            return right.has_value()
                ? std::optional<bool>(compareValues(*left, *right, RelationalOperator::Less))
                : std::nullopt;
        }
        if (consume(">")) {
            const auto right = parseRight(">");
            return right.has_value()
                ? std::optional<bool>(compareValues(*left, *right, RelationalOperator::Greater))
                : std::nullopt;
        }

        return left->truthy();
    }

    std::optional<ConditionValue> parseValue() {
        skipWhitespace();
        if (position_ >= expression_.size()) {
            return std::nullopt;
        }

        const auto ch = expression_[position_];
        if (ch == '\'' || ch == '"') {
            return parseString();
        }
        if (std::isdigit(static_cast<unsigned char>(ch)) ||
            (ch == '-' && position_ + 1 < expression_.size() &&
             std::isdigit(static_cast<unsigned char>(expression_[position_ + 1]))))
        {
            return parseNumber();
        }

        const auto identifier = parseIdentifier();
        if (identifier.empty()) {
            return std::nullopt;
        }

        if (identifier == "true") {
            return ConditionValue(true);
        }
        if (identifier == "false") {
            return ConditionValue(false);
        }
        if (identifier == "null") {
            return ConditionValue();
        }

        return context_ != nullptr
            ? resolveConditionValue(identifier, *context_)
            : ConditionValue();
    }

    std::optional<ConditionValue> parseString() {
        const auto start = position_;
        const auto quote = expression_[position_++];
        std::string result;

        while (position_ < expression_.size()) {
            const auto ch = expression_[position_++];
            if (ch == quote) {
                return ConditionValue(std::move(result));
            }
            if (ch == '\\' && position_ < expression_.size()) {
                const auto escaped = expression_[position_++];
                switch (escaped) {
                    case 'n':
                        result += '\n';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    default:
                        result += escaped;
                        break;
                }
            } else {
                result += ch;
            }
        }

        fail("Unterminated string literal", start);
        return std::nullopt;
    }

    std::optional<ConditionValue> parseNumber() {
        skipWhitespace();
        const auto start = position_;
        if (expression_[position_] == '-') ++position_;

        while (position_ < expression_.size() &&
               std::isdigit(static_cast<unsigned char>(expression_[position_]))) {
            ++position_;
        }
        if (position_ < expression_.size() && expression_[position_] == '.') {
            ++position_;
            const auto fractionalStart = position_;
            while (position_ < expression_.size() &&
                   std::isdigit(static_cast<unsigned char>(expression_[position_]))) {
                ++position_;
            }
            if (fractionalStart == position_) {
                fail("Expected digits after decimal point", position_);
                return std::nullopt;
            }
        }

        try {
            return ConditionValue(
                std::stod(std::string(expression_.substr(start, position_ - start)))
            );
        } catch (...) {
            fail("Invalid number literal", start);
            return std::nullopt;
        }
    }

    std::string parseIdentifier() {
        skipWhitespace();
        const auto start = position_;

        while (position_ < expression_.size()) {
            const auto ch = expression_[position_];
            if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '.') {
                ++position_;
            } else {
                break;
            }
        }

        return std::string(expression_.substr(start, position_ - start));
    }

    bool consume(std::string_view token) {
        skipWhitespace();
        if (expression_.substr(position_, token.size()) != token) {
            return false;
        }

        previousTokenPosition_ = position_;
        position_ += token.size();

        return true;
    }

    void skipWhitespace() {
        while (position_ < expression_.size() &&
               std::isspace(static_cast<unsigned char>(expression_[position_]))) {
            ++position_;
        }
    }

    void fail(std::string message, std::size_t position) {
        if (!error_.has_value()) {
            error_ = ConditionExpressionError{
                .message = std::move(message),
                .position = position
            };
        }
    }

    std::string_view expression_;
    const RenderContext* context_ = nullptr;
    size_t position_ = 0;
    size_t previousTokenPosition_ = 0;
    std::optional<ConditionExpressionError> error_;
};

} // namespace

std::optional<ConditionExpressionError> validateConditionExpression(
    std::string_view expression
) {
    ConditionParser parser(expression);
    parser.parse();
    return parser.error();
}

bool evaluateCondition(
    std::string_view expression,
    const RenderContext& context
) {
    return ConditionParser(expression, &context).parse().value_or(false);
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