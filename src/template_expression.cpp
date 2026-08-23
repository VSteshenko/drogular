#include <drogular/template_expression.hpp>
#include <drogular/render_context.hpp>

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

namespace drogular::template_expression {

std::vector<std::int64_t> ExpressionRange::materialize() const {
    std::vector<std::int64_t> result;
    if (step == 0) {
        return result;
    }

    const auto within = [this](std::int64_t value) {
        if (step > 0) {
            return upperInclusive ? value <= end : value < end;
        }
        return upperInclusive ? value >= end : value > end;
    };

    for (auto value = start; within(value);) {
        result.push_back(value);

        if ((step > 0 && value > std::numeric_limits<std::int64_t>::max() - step) ||
            (step < 0 && value < std::numeric_limits<std::int64_t>::min() - step)
        ) {
            break;
        }
        value += step;
    }

    return result;
}

ExpressionValue::ExpressionValue(bool value) : value_(value) {}
ExpressionValue::ExpressionValue(double value) : value_(value) {}
ExpressionValue::ExpressionValue(std::string value) : value_(std::move(value)) {}
ExpressionValue::ExpressionValue(Json::Value value) : value_(std::move(value)) {}
ExpressionValue::ExpressionValue(ArrayPtr value) : value_(std::move(value)) {}
ExpressionValue::ExpressionValue(ExpressionRange value) : value_(std::move(value)) {}

bool ExpressionValue::truthy() const {
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
    if (const auto value = std::get_if<ArrayPtr>(&value_)) {
        return *value && !(*value)->values.empty();
    }
    if (const auto value = std::get_if<ExpressionRange>(&value_)) {
        if (value->step > 0) {
            return value->upperInclusive
                ? value->start <= value->end
                : value->start < value->end;
        }
        return value->upperInclusive
            ? value->start >= value->end
            : value->start > value->end;
    }

    return false;
}

bool ExpressionValue::isNull() const {
    if (std::holds_alternative<std::monostate>(value_)) {
        return true;
    }
    if (const auto value = std::get_if<Json::Value>(&value_)) {
        return value->isNull();
    }

    return false;
}

std::optional<double> ExpressionValue::number() const {
    if (const auto value = std::get_if<double>(&value_)) {
        return *value;
    }
    if (const auto value = std::get_if<Json::Value>(&value_);
        value && value->isNumeric()
    ) {
        return value->asDouble();
    }

    return std::nullopt;
}

std::optional<std::string> ExpressionValue::string() const {
    if (const auto value = std::get_if<std::string>(&value_)) {
        return *value;
    }
    if (const auto value = std::get_if<Json::Value>(&value_);
        value && value->isString()) {
        return value->asString();
    }

    return std::nullopt;
}

std::optional<bool> ExpressionValue::boolean() const {
    if (const auto value = std::get_if<bool>(&value_)) {
        return *value;
    }
    if (const auto value = std::get_if<Json::Value>(&value_);
        value && value->isBool()) {
        return value->asBool();
    }

    return std::nullopt;
}

ExpressionValue::ArrayPtr ExpressionValue::array() const {
    if (const auto value = std::get_if<ArrayPtr>(&value_)) {
        return *value;
    }

    return nullptr;
}

const ExpressionRange* ExpressionValue::range() const {
    return std::get_if<ExpressionRange>(&value_);
}

const ExpressionValue::Storage& ExpressionValue::storage() const noexcept {
    return value_;
}

namespace {

bool equalValues(const ExpressionValue& left, const ExpressionValue& right) {
    if (left.isNull() || right.isNull()) {
        return left.isNull() && right.isNull();
    }
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

template <typename Predicate>
bool compareValues(const ExpressionValue& left, const ExpressionValue& right, Predicate predicate) {
    if (const auto lhs = left.number()) {
        if (const auto rhs = right.number()) {
            return predicate(*lhs, *rhs);
        }
    }
    if (const auto lhs = left.string()) {
        if (const auto rhs = right.string()) {
            return predicate(*lhs, *rhs);
        }
    }

    return false;
}

std::optional<std::int64_t> integerValue(const ExpressionValue& value) {
    const auto number = value.number();
    if (!number || !std::isfinite(*number) || std::trunc(*number) != *number) {
        return std::nullopt;
    }
    if (*number < static_cast<double>(std::numeric_limits<std::int64_t>::min()) ||
        *number > static_cast<double>(std::numeric_limits<std::int64_t>::max())
    ) {
        return std::nullopt;
    }

    return static_cast<std::int64_t>(*number);
}

ExpressionValue evaluateNode(const Expression& expression, const RenderContext& context) {
    if (const auto* node = std::get_if<LiteralExpression>(&expression.node)) {
        return node->value;
    }
    if (const auto* node = std::get_if<VariableExpression>(&expression.node)) {
        return resolve(node->path, context);
    }
    if (const auto* node = std::get_if<UnaryExpression>(&expression.node)) {
        const auto value = evaluateNode(*node->operand, context);
        if (node->op == UnaryOperator::Not) {
            return ExpressionValue(!value.truthy());
        }
        if (const auto number = value.number()) {
            return ExpressionValue(-*number);
        }
        return ExpressionValue();
    }
    if (const auto* node = std::get_if<ListExpression>(&expression.node)) {
        auto array = std::make_shared<ExpressionArray>();
        array->values.reserve(node->elements.size());
        for (const auto& element : node->elements) {
            array->values.push_back(evaluateNode(*element, context));
        }
        return ExpressionValue(std::move(array));
    }
    if (const auto* node = std::get_if<RangeExpression>(&expression.node)) {
        const auto start = integerValue(evaluateNode(*node->start, context));
        const auto end = integerValue(evaluateNode(*node->end, context));
        if (!start || !end) {
            return ExpressionValue();
        }

        std::int64_t step = *start <= *end ? 1 : -1;
        if (node->step) {
            const auto explicitStep =
                integerValue(evaluateNode(*node->step, context));
            if (!explicitStep || *explicitStep == 0) {
                return ExpressionValue();
            }
            step = *explicitStep;
        }

        if ((*start < *end && step < 0) || (*start > *end && step > 0)) {
            return ExpressionValue();
        }

        return ExpressionValue(ExpressionRange{
            .start = *start,
            .end = *end,
            .step = step,
            .upperInclusive = node->upperInclusive
        });
    }

    const auto* node = std::get_if<BinaryExpression>(&expression.node);
    if (node == nullptr) {
        return ExpressionValue();
    }

    if (node->op == BinaryOperator::And) {
        const auto left = evaluateNode(*node->left, context);
        if (!left.truthy()) {
            return ExpressionValue(false);
        }
        return ExpressionValue(evaluateNode(*node->right, context).truthy());
    }
    if (node->op == BinaryOperator::Or) {
        const auto left = evaluateNode(*node->left, context);
        if (left.truthy()) {
            return ExpressionValue(true);
        }
        return ExpressionValue(evaluateNode(*node->right, context).truthy());
    }

    const auto left = evaluateNode(*node->left, context);
    const auto right = evaluateNode(*node->right, context);

    switch (node->op) {
        case BinaryOperator::Add:
        case BinaryOperator::Subtract:
        case BinaryOperator::Multiply:
        case BinaryOperator::Divide: {
            const auto lhs = left.number();
            const auto rhs = right.number();
            if (!lhs || !rhs) {
                return ExpressionValue();
            }
            if (node->op == BinaryOperator::Add) {
                return ExpressionValue(*lhs + *rhs);
            }
            if (node->op == BinaryOperator::Subtract) {
                return ExpressionValue(*lhs - *rhs);
            }
            if (node->op == BinaryOperator::Multiply) {
                return ExpressionValue(*lhs * *rhs);
            }
            if (*rhs == 0.0) {
                return ExpressionValue();
            }
            return ExpressionValue(*lhs / *rhs);
        }

        case BinaryOperator::Equal:
            return ExpressionValue(equalValues(left, right));

        case BinaryOperator::NotEqual:
            return ExpressionValue(!equalValues(left, right));

        case BinaryOperator::Less:
            return ExpressionValue(compareValues(
                left,
                right,
                [](const auto& lhs, const auto& rhs) {
                    return lhs < rhs;
                }
            ));

        case BinaryOperator::LessEqual:
            return ExpressionValue(compareValues(
                left,
                right,
                [](const auto& lhs, const auto& rhs) {
                    return lhs <= rhs;
                }
            ));

        case BinaryOperator::Greater:
            return ExpressionValue(compareValues(
                left,
                right,
                [](const auto& lhs, const auto& rhs) {
                    return lhs > rhs;
                }
            ));

        case BinaryOperator::GreaterEqual:
            return ExpressionValue(compareValues(
                left,
                right,
                [](const auto& lhs, const auto& rhs) {
                    return lhs >= rhs;
                }
            ));

        case BinaryOperator::And:
        case BinaryOperator::Or:
            break;
    }
    return ExpressionValue();
}

} // namespace

ExpressionValue evaluate(const Expression& expression, const RenderContext& context) {
    return evaluateNode(expression, context);
}

ExpressionValue evaluate(std::string_view source, const RenderContext& context) {
    const auto result = parse(source);
    if (!result) {
        return ExpressionValue();
    }

    return evaluate(*result.expression, context);
}

ExpressionValue resolve(std::string_view path, const RenderContext& context) {
    const auto key = std::string(path);
    if (key.empty()) {
        return ExpressionValue();
    }

    if (const auto value = context.get<bool>(key)) {
        return ExpressionValue(*value);
    }
    if (const auto value = context.get<int>(key)) {
        return ExpressionValue(static_cast<double>(*value));
    }
    if (const auto value = context.get<double>(key)) {
        return ExpressionValue(*value);
    }
    if (const auto value = context.get<std::string>(key)) {
        return ExpressionValue(*value);
    }

    const auto separator = key.find('.');
    const auto rootKey = separator == std::string::npos
        ? key
        : key.substr(0, separator);
    const auto root = context.get<Json::Value>(rootKey);
    if (!root.has_value()) {
        return ExpressionValue();
    }
    if (separator == std::string::npos) {
        return ExpressionValue(*root);
    }

    Json::Value current = *root;
    std::size_t start = separator + 1;
    while (start <= key.size()) {
        const auto end = key.find('.', start);
        const auto member =
            key.substr(start, end == std::string::npos
                ? std::string::npos
                : end - start);
        if (member.empty() || !current.isObject() || !current.isMember(member)) {
            return ExpressionValue();
        }
        current = current[member];
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }

    return ExpressionValue(std::move(current));
}

} // namespace drogular::template_expression