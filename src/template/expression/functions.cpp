#include <drogular/template/expression/functions.hpp>

#include <utility>

namespace drogular::template_expression {

void ExpressionFunctionRegistry::registerFunction(
    std::shared_ptr<const ExpressionFunction> function
) {
    if (!function || function->name().empty()) {
        return;
    }
    functions_[std::string(function->name())] = std::move(function);
}

const ExpressionFunction* ExpressionFunctionRegistry::find(
    std::string_view name
) const noexcept {
    const auto found = functions_.find(std::string(name));
    return found == functions_.end() ? nullptr : found->second.get();
}

namespace {

class CountFunction final : public ExpressionFunction {
public:
    [[nodiscard]] std::string_view name() const noexcept override {
        return "count";
    }

    [[nodiscard]] ExpressionValue invoke(
        const ExpressionValue& self,
        std::span<const ExpressionValue> arguments
    ) const override {
        if (!arguments.empty()) {
            return ExpressionValue();
        }
        const auto iterable = self.iterable();
        return iterable
            ? ExpressionValue(static_cast<double>(iterable->size()))
            : ExpressionValue();
    }
};

class EmptyFunction final : public ExpressionFunction {
public:
    [[nodiscard]] std::string_view name() const noexcept override {
        return "empty";
    }

    [[nodiscard]] ExpressionValue invoke(
        const ExpressionValue& self,
        std::span<const ExpressionValue> arguments
    ) const override {
        if (!arguments.empty()) {
            return ExpressionValue();
        }
        const auto iterable = self.iterable();
        return iterable ? ExpressionValue(iterable->empty()) : ExpressionValue();
    }
};

class FirstFunction final : public ExpressionFunction {
public:
    [[nodiscard]] std::string_view name() const noexcept override {
        return "first";
    }

    [[nodiscard]] ExpressionValue invoke(
        const ExpressionValue& self,
        std::span<const ExpressionValue> arguments
    ) const override {
        if (!arguments.empty()) {
            return ExpressionValue();
        }
        const auto iterable = self.iterable();
        return iterable && !iterable->empty()
            ? iterable->at(0)
            : ExpressionValue();
    }
};

class LastFunction final : public ExpressionFunction {
public:
    [[nodiscard]] std::string_view name() const noexcept override {
        return "last";
    }

    [[nodiscard]] ExpressionValue invoke(
        const ExpressionValue& self,
        std::span<const ExpressionValue> arguments
    ) const override {
        if (!arguments.empty()) {
            return ExpressionValue();
        }
        const auto iterable = self.iterable();
        return iterable && !iterable->empty()
            ? iterable->at(iterable->size() - 1)
            : ExpressionValue();
    }
};

class ContainsFunction final : public ExpressionFunction {
public:
    [[nodiscard]] std::string_view name() const noexcept override {
        return "contains";
    }

    [[nodiscard]] ExpressionValue invoke(
        const ExpressionValue& self,
        std::span<const ExpressionValue> arguments
    ) const override {
        if (arguments.size() != 1) {
            return ExpressionValue();
        }
        return ExpressionValue(self.contains(arguments.front()));
    }
};

ExpressionFunctionRegistry makeBuiltinRegistry() {
    ExpressionFunctionRegistry registry;
    registry.registerFunction(std::make_shared<CountFunction>());
    registry.registerFunction(std::make_shared<EmptyFunction>());
    registry.registerFunction(std::make_shared<FirstFunction>());
    registry.registerFunction(std::make_shared<LastFunction>());
    registry.registerFunction(std::make_shared<ContainsFunction>());
    return registry;
}

} // namespace

const ExpressionFunctionRegistry& builtinFunctionRegistry() {
    static const auto registry = makeBuiltinRegistry();
    return registry;
}

} // namespace drogular::template_expression