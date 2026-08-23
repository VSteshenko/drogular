#pragma once

#include <json/json.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace drogular::template_expression {

class ExpressionValue;
struct ExpressionArray;
class ExpressionIterable;

/**
 * Evaluated integer range.
 *
 * The upper bound may be inclusive (`..`) or exclusive (`..<`). The step is
 * always non-zero and has a direction compatible with start/end.
 */
struct ExpressionRange {
    std::int64_t start = 0;
    std::int64_t end = 0;
    std::int64_t step = 1;
    bool upperInclusive = true;

    [[nodiscard]] std::vector<std::int64_t> materialize() const;
};

/**
 * Runtime value produced by the template expression engine.
 *
 * Json::Value is intentionally retained as a storage alternative so existing
 * RenderContext JSON objects and arrays can flow through the expression engine
 * without conversion. Native arrays and ranges provide expression-owned
 * iterable values for list/range literals.
 */
class ExpressionValue {
public:
    using ArrayPtr = std::shared_ptr<const ExpressionArray>;
    using Storage = std::variant<
        std::monostate,
        bool,
        double,
        std::string,
        Json::Value,
        ArrayPtr,
        ExpressionRange
    >;

    ExpressionValue() = default;
    explicit ExpressionValue(bool value);
    explicit ExpressionValue(double value);
    explicit ExpressionValue(std::string value);
    explicit ExpressionValue(Json::Value value);
    explicit ExpressionValue(ArrayPtr value);
    explicit ExpressionValue(ExpressionRange value);

    [[nodiscard]] bool truthy() const;
    [[nodiscard]] bool isNull() const;
    [[nodiscard]] std::optional<double> number() const;
    [[nodiscard]] std::optional<std::string> string() const;
    [[nodiscard]] std::optional<bool> boolean() const;
    [[nodiscard]] ArrayPtr array() const;
    [[nodiscard]] const ExpressionRange* range() const;
    [[nodiscard]] const Storage& storage() const noexcept;

    /** Returns true for native lists, ranges, and JSON arrays. */
    [[nodiscard]] bool isIterable() const;

    /**
     * Returns a uniform indexed iterable view, or nullptr when this value is
     * not iterable. Range elements are generated lazily by the view.
     */
    [[nodiscard]] std::shared_ptr<const ExpressionIterable> iterable() const;

private:
    Storage value_;
};

struct ExpressionArray {
    std::vector<ExpressionValue> values;
};

/**
 * Uniform indexed view over expression-owned lists, ranges, and JSON arrays.
 *
 * The iterable retains the underlying ExpressionValue, so returned elements
 * remain valid even when the source value was produced by a temporary AST
 * evaluation. Range elements are generated on demand and are not materialized.
 */
class ExpressionIterable {
public:
    explicit ExpressionIterable(ExpressionValue value);

    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] ExpressionValue at(std::size_t index) const;

private:
    ExpressionValue value_;
};

} // namespace drogular::template_expression