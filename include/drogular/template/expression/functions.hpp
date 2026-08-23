#pragma once

#include <drogular/template/expression/value.hpp>

#include <functional>
#include <memory>
#include <shared_mutex>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace drogular::template_expression {

class BindingContext;

class ExpressionFunction {
public:
    virtual ~ExpressionFunction() = default;
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual ExpressionValue invoke(
        const ExpressionValue& self,
        std::span<const ExpressionValue> arguments
    ) const = 0;
};

using ExpressionFunctionCallback = std::function<ExpressionValue(
    std::span<const ExpressionValue>, const BindingContext&)>;
using ExpressionMethodCallback = std::function<ExpressionValue(
    const ExpressionValue&, std::span<const ExpressionValue>, const BindingContext&)>;

/** Application-local registry. Registration is intended for startup only. */
class ExpressionFunctionRegistry {
public:
    explicit ExpressionFunctionRegistry(
        const ExpressionFunctionRegistry* parent = nullptr
    ) : parent_(parent) {}

    bool registerFunction(std::string name, ExpressionFunctionCallback callback);
    bool registerMethod(std::string name, ExpressionMethodCallback callback);
    bool registerMethod(std::shared_ptr<const ExpressionFunction> function);

    [[nodiscard]] ExpressionFunctionCallback findFunction(std::string_view name) const;
    [[nodiscard]] ExpressionMethodCallback findMethod(std::string_view name) const;

    /** Prevents further registration. Safe to call repeatedly. */
    void freeze();
    [[nodiscard]] bool frozen() const;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, ExpressionFunctionCallback> functions_;
    std::unordered_map<std::string, ExpressionMethodCallback> methods_;
    bool frozen_ = false;
    const ExpressionFunctionRegistry* parent_ = nullptr;
};

[[nodiscard]] const ExpressionFunctionRegistry& builtinFunctionRegistry();

} // namespace drogular::template_expression