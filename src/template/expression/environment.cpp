#include <drogular/template/expression/environment.hpp>
#include <drogular/render_context.hpp>
#include <drogular/services.hpp>

namespace drogular::template_expression {

ExpressionEnvironment::ExpressionEnvironment(const BindingContext& bindings)
    : bindings_(&bindings)
{
    if (const auto* services = bindings.renderContext().services()) {
        applicationFunctions_ = &services->expressionFunctions();
    }
}

const BindingContext& ExpressionEnvironment::bindings() const noexcept {
    return *bindings_;
}

ExpressionValue ExpressionEnvironment::invokeFunction(
    std::string_view name,
    std::span<const ExpressionValue> arguments
) const noexcept {
    try {
        if (applicationFunctions_) {
            if (auto callback = applicationFunctions_->findFunction(name)) {
                return callback(arguments, *bindings_);
            }
            return ExpressionValue();
        }
        if (auto callback = builtinFunctionRegistry().findFunction(name)) {
            return callback(arguments, *bindings_);
        }
    } catch (...) {
        return ExpressionValue();
    }

    return ExpressionValue();
}

ExpressionValue ExpressionEnvironment::invokeMethod(
    std::string_view name,
    const ExpressionValue& self,
    std::span<const ExpressionValue> arguments
) const noexcept {
    try {
        if (applicationFunctions_) {
            if (auto callback = applicationFunctions_->findMethod(name)) {
                return callback(self, arguments, *bindings_);
            }
            return ExpressionValue();
        }
        if (auto callback = builtinFunctionRegistry().findMethod(name)) {
            return callback(self, arguments, *bindings_);
        }
    } catch (...) {
        return ExpressionValue();
    }

    return ExpressionValue();
}

} // namespace drogular::template_expression