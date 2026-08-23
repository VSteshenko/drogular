#pragma once

#include <drogular/template/expression/value.hpp>

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace drogular::template_expression {

/** Runtime method callable on an ExpressionValue receiver. */
class ExpressionFunction {
public:
    virtual ~ExpressionFunction() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;

    [[nodiscard]] virtual ExpressionValue invoke(
        const ExpressionValue& self,
        std::span<const ExpressionValue> arguments
    ) const = 0;
};

/** Registry used to resolve expression methods by name. */
class ExpressionFunctionRegistry {
public:
    void registerFunction(std::shared_ptr<const ExpressionFunction> function);

    [[nodiscard]] const ExpressionFunction* find(
        std::string_view name
    ) const noexcept;

private:
    std::unordered_map<std::string, std::shared_ptr<const ExpressionFunction>> functions_;
};

/** Built-in collection-function registry used by the evaluator. */
[[nodiscard]] const ExpressionFunctionRegistry& builtinFunctionRegistry();

} // namespace drogular::template_expression