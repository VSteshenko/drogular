#include <drogular/template/expression/functions.hpp>
#include <drogular/template/expression/binding_context.hpp>

#include <mutex>
#include <utility>

namespace drogular::template_expression {

namespace {

bool validFunctionName(std::string_view name) {
    if (name.empty()) {
        return false;
    }
    const auto first = static_cast<unsigned char>(name.front());
    if (!(std::isalpha(first) || name.front() == '_')) {
        return false;
    }
    for (const auto ch : name.substr(1)) {
        const auto value = static_cast<unsigned char>(ch);
        if (!(std::isalnum(value) || ch == '_')) {
            return false;
        }
    }

    return name != "true" && name != "false" && name != "null";
}

} // namespace

bool ExpressionFunctionRegistry::registerFunction(
    std::string name,
    ExpressionFunctionCallback callback
) {
    if (!validFunctionName(name) || !callback) {
        return false;
    }
    if (parent_ && (parent_->findFunction(name) || parent_->findMethod(name))) {
        return false;
    }
    std::unique_lock lock(mutex_);
    if (frozen_ || functions_.contains(name) || methods_.contains(name)) {
        return false;
    }
    functions_.emplace(std::move(name), std::move(callback));

    return true;
}

bool ExpressionFunctionRegistry::registerMethod(
    std::string name,
    ExpressionMethodCallback callback
) {
    if (!validFunctionName(name) || !callback) {
        return false;
    }
    if (parent_ && (parent_->findFunction(name) || parent_->findMethod(name))) {
        return false;
    }
    std::unique_lock lock(mutex_);
    if (frozen_ || methods_.contains(name) || functions_.contains(name)) {
        return false;
    }
    methods_.emplace(std::move(name), std::move(callback));

    return true;
}

bool ExpressionFunctionRegistry::registerMethod(
    std::shared_ptr<const ExpressionFunction> function
) {
    if (!function || function->name().empty()) return false;
    auto name = std::string(function->name());
    return registerMethod(
        std::move(name),
        [function = std::move(function)](
            const ExpressionValue& self,
            std::span<const ExpressionValue> arguments,
            const BindingContext&
        ) { return function->invoke(self, arguments); }
    );
}

ExpressionFunctionCallback ExpressionFunctionRegistry::findFunction(
    std::string_view name
) const {
    std::shared_lock lock(mutex_);
    const auto found = functions_.find(std::string(name));
    if (found != functions_.end()) {
        return found->second;
    }
    lock.unlock();

    return parent_ ? parent_->findFunction(name) : ExpressionFunctionCallback{};
}

ExpressionMethodCallback ExpressionFunctionRegistry::findMethod(
    std::string_view name
) const {
    std::shared_lock lock(mutex_);
    const auto found = methods_.find(std::string(name));
    if (found != methods_.end()) {
        return found->second;
    }
    lock.unlock();

    return parent_ ? parent_->findMethod(name) : ExpressionMethodCallback{};
}

void ExpressionFunctionRegistry::freeze() {
    std::unique_lock lock(mutex_);
    frozen_ = true;
}

bool ExpressionFunctionRegistry::frozen() const {
    std::shared_lock lock(mutex_);
    return frozen_;
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
        return iterable
            ? ExpressionValue(iterable->empty())
            : ExpressionValue();
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

void registerBuiltins(ExpressionFunctionRegistry& registry) {
    registry.registerMethod(std::make_shared<CountFunction>());
    registry.registerMethod(std::make_shared<EmptyFunction>());
    registry.registerMethod(std::make_shared<FirstFunction>());
    registry.registerMethod(std::make_shared<LastFunction>());
    registry.registerMethod(std::make_shared<ContainsFunction>());
    registry.freeze();
}

} // namespace

const ExpressionFunctionRegistry& builtinFunctionRegistry() {
    static ExpressionFunctionRegistry registry;
    static std::once_flag once;
    std::call_once(once, [] { registerBuiltins(registry); });

    return registry;
}

} // namespace drogular::template_expression