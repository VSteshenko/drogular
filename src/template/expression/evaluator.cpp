#include <drogular/template/expression/evaluator.hpp>
#include <drogular/template/expression/functions.hpp>
#include <drogular/template/expression/parser.hpp>
#include <drogular/render_context.hpp>

#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

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

bool ExpressionValue::equals(const ExpressionValue& other) const {
    if (isNull() || other.isNull()) {
        return isNull() && other.isNull();
    }
    if (const auto lhs = number()) {
        if (const auto rhs = other.number()) {
            return *lhs == *rhs;
        }
    }
    if (const auto lhs = string()) {
        if (const auto rhs = other.string()) {
            return *lhs == *rhs;
        }
    }
    if (const auto lhs = boolean()) {
        if (const auto rhs = other.boolean()) {
            return *lhs == *rhs;
        }
    }

    return false;
}

ExpressionValue ExpressionValue::member(std::string_view name) const {
    if (const auto* json = std::get_if<Json::Value>(&value_);
        json && json->isObject()
    ) {
        const auto key = std::string(name);
        if (json->isMember(key)) {
            return ExpressionValue((*json)[key]);
        }
    }

    return ExpressionValue();
}

bool ExpressionValue::isIterable() const {
    if (std::holds_alternative<ArrayPtr>(value_)) {
        return true;
    }
    if (std::holds_alternative<ExpressionRange>(value_)) {
        return true;
    }
    if (const auto value = std::get_if<Json::Value>(&value_)) {
        return value->isArray();
    }

    return false;
}

std::shared_ptr<const ExpressionIterable> ExpressionValue::iterable() const {
    if (!isIterable()) {
        return nullptr;
    }

    return std::make_shared<ExpressionIterable>(*this);
}

ExpressionIterable::ExpressionIterable(ExpressionValue value)
    : value_(std::move(value)) {
}

std::size_t ExpressionIterable::size() const {
    if (const auto array = value_.array()) {
        return array->values.size();
    }
    if (const auto* json = std::get_if<Json::Value>(&value_.storage());
        json && json->isArray()) {
        return json->size();
    }
    if (const auto* range = value_.range()) {
        if (range->step == 0) {
            return 0;
        }
        if (range->step > 0) {
            if (range->upperInclusive
                ? range->start > range->end
                : range->start >= range->end
            ) {
                return 0;
            }
        } else if (range->upperInclusive
            ? range->start < range->end
            : range->start <= range->end
        ) {
            return 0;
        }

        const auto distance = range->step > 0
            ? static_cast<std::uint64_t>(range->end) -
                static_cast<std::uint64_t>(range->start)
            : static_cast<std::uint64_t>(range->start) -
                static_cast<std::uint64_t>(range->end);
        const auto stepMagnitude = range->step > 0
            ? static_cast<std::uint64_t>(range->step)
            : std::uint64_t{0} - static_cast<std::uint64_t>(range->step);

        std::uint64_t count = 0;
        if (range->upperInclusive) {
            const auto quotient = distance / stepMagnitude;
            if (quotient == std::numeric_limits<std::uint64_t>::max()) {
                return std::numeric_limits<std::size_t>::max();
            }
            count = quotient + 1;
        } else if (distance != 0) {
            count = (distance - 1) / stepMagnitude + 1;
        }

        if constexpr (sizeof(std::size_t) < sizeof(std::uint64_t)) {
            if (count > static_cast<std::uint64_t>(
                    std::numeric_limits<std::size_t>::max())) {
                return std::numeric_limits<std::size_t>::max();
            }
        }
        return static_cast<std::size_t>(count);
    }
    return 0;
}

bool ExpressionIterable::empty() const {
    return size() == 0;
}

ExpressionValue ExpressionIterable::at(std::size_t index) const {
    if (const auto array = value_.array()) {
        if (index < array->values.size()) {
            return array->values[index];
        }
        return ExpressionValue();
    }
    if (const auto* json = std::get_if<Json::Value>(&value_.storage());
        json && json->isArray()) {
        if (index < json->size()) {
            return ExpressionValue((*json)[static_cast<Json::ArrayIndex>(index)]);
        }
        return ExpressionValue();
    }
    if (const auto* range = value_.range()) {
        if (index >= size()) {
            return ExpressionValue();
        }
        const auto result = static_cast<long double>(range->start) +
            static_cast<long double>(range->step) *
            static_cast<long double>(index);
        return ExpressionValue(static_cast<double>(result));
    }
    return ExpressionValue();
}

bool ExpressionValue::contains(const ExpressionValue& candidate) const {
    if (const auto* rangeValue = range()) {
        const auto number = candidate.number();
        if (!number || !std::isfinite(*number) || std::trunc(*number) != *number ||
            *number < static_cast<double>(std::numeric_limits<std::int64_t>::min()) ||
            *number > static_cast<double>(std::numeric_limits<std::int64_t>::max())
        ) {
            return false;
        }
        const auto value = static_cast<std::int64_t>(*number);
        if (rangeValue->step == 0) {
            return false;
        }
        if (rangeValue->step > 0) {
            if (value < rangeValue->start ||
                (rangeValue->upperInclusive
                    ? value > rangeValue->end
                    : value >= rangeValue->end)
            ) {
                return false;
            }
            const auto distance = static_cast<std::uint64_t>(value) -
                static_cast<std::uint64_t>(rangeValue->start);
            return distance % static_cast<std::uint64_t>(rangeValue->step) == 0;
        }
        if (value > rangeValue->start ||
            (rangeValue->upperInclusive
                ? value < rangeValue->end
                : value <= rangeValue->end)
        ) {
            return false;
        }
        const auto distance = static_cast<std::uint64_t>(rangeValue->start) -
            static_cast<std::uint64_t>(value);
        const auto stepMagnitude = std::uint64_t{0} -
            static_cast<std::uint64_t>(rangeValue->step);
        return distance % stepMagnitude == 0;
    }

    const auto values = iterable();
    if (!values) {
        return false;
    }
    for (std::size_t index = 0; index < values->size(); ++index) {
        if (candidate.equals(values->at(index))) {
            return true;
        }
    }
    return false;
}

namespace {

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

ExpressionValue evaluateNode(const Expression& expression, const BindingContext& context) {
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
    if (const auto* node = std::get_if<MemberAccessExpression>(&expression.node)) {
        return evaluateNode(*node->object, context).member(node->member);
    }
    if (const auto* node = std::get_if<MethodCallExpression>(&expression.node)) {
        const auto self = evaluateNode(*node->object, context);
        std::vector<ExpressionValue> arguments;
        arguments.reserve(node->arguments.size());
        for (const auto& argument : node->arguments) {
            arguments.push_back(evaluateNode(*argument, context));
        }
        const auto* function = builtinFunctionRegistry().find(node->method);
        if (!function) {
            return ExpressionValue();
        }
        return function->invoke(self, arguments);
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
            return ExpressionValue(left.equals(right));

        case BinaryOperator::NotEqual:
            return ExpressionValue(!left.equals(right));

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

        case BinaryOperator::In:
            return ExpressionValue(right.contains(left));

        case BinaryOperator::NotIn:
            return ExpressionValue(!right.contains(left));

        case BinaryOperator::And:
        case BinaryOperator::Or:
            break;
    }

    return ExpressionValue();
}

} // namespace

ExpressionValue evaluate(const Expression& expression, const BindingContext& context) {
    return evaluateNode(expression, context);
}

ExpressionValue evaluate(const Expression& expression, const RenderContext& context) {
    const BindingContext bindings(context);
    return evaluate(expression, bindings);
}

ExpressionValue evaluate(std::string_view source, const BindingContext& context) {
    const auto result = parse(source);
    if (!result) {
        return ExpressionValue();
    }

    return evaluate(*result.expression, context);
}

ExpressionValue evaluate(std::string_view source, const RenderContext& context) {
    const BindingContext bindings(context);
    return evaluate(source, bindings);
}

ExpressionValue resolve(std::string_view path, const BindingContext& context) {
    return context.resolve(path);
}

ExpressionValue resolve(std::string_view path, const RenderContext& context) {
    const BindingContext bindings(context);
    return bindings.resolve(path);
}

} // namespace drogular::template_expression