#pragma once

#include <drogular/template/expression/binding_context.hpp>
#include <drogular/template/expression/functions.hpp>

#include <span>
#include <string_view>

namespace drogular::template_expression {

/** Runtime services used while evaluating one expression tree. */
class ExpressionEnvironment {
public:
    explicit ExpressionEnvironment(const BindingContext& bindings);

    [[nodiscard]] const BindingContext& bindings() const noexcept;

    [[nodiscard]] ExpressionValue invokeFunction(
        std::string_view name,
        std::span<const ExpressionValue> arguments
    ) const noexcept;

    [[nodiscard]] ExpressionValue invokeMethod(
        std::string_view name,
        const ExpressionValue& self,
        std::span<const ExpressionValue> arguments
    ) const noexcept;

private:
    const BindingContext* bindings_ = nullptr;
    const ExpressionFunctionRegistry* applicationFunctions_ = nullptr;
};

} // namespace drogular::template_expression