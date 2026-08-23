#include <drogular/template/expression/binding_context.hpp>
#include <drogular/render_context.hpp>

#include <json/json.h>

#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <utility>

namespace drogular::template_expression {
namespace {

ExpressionValue resolveRenderContext(
    std::string_view path,
    const RenderContext& context
) {
    const auto key = std::string(path);
    if (key.empty()) {
        return ExpressionValue();
    }

    if (const auto value = context.get<ExpressionValue>(key)) {
        return *value;
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
    if (const auto values = context.get<std::vector<std::string>>(key)) {
        auto array = std::make_shared<ExpressionArray>();
        array->values.reserve(values->size());
        for (const auto& value : *values) {
            array->values.emplace_back(value);
        }
        return ExpressionValue(std::move(array));
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

    ExpressionValue current(*root);
    std::size_t start = separator + 1;
    while (start <= key.size()) {
        const auto end = key.find('.', start);
        const auto member = key.substr(
            start,
            end == std::string::npos ? std::string::npos : end - start
        );
        if (member.empty()) {
            return ExpressionValue();
        }
        current = current.member(member);
        if (current.isNull()) {
            return ExpressionValue();
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }

    return current;
}

} // namespace

BindingContext::BindingContext(const RenderContext& renderContext)
    : renderContext_(&renderContext) {
}

BindingContext::BindingContext(const BindingContext& parent)
    : renderContext_(&parent.renderContext()),
      parent_(&parent) {
}

BindingContext BindingContext::createChild() const {
    return BindingContext(*this);
}

bool BindingContext::define(
    std::string name,
    ExpressionValue value,
    BindingMutability mutability
) {
    if (name.empty() || bindings_.contains(name)) {
        return false;
    }

    bindings_.emplace(
        std::move(name),
        Binding{
            .value = std::move(value),
            .mutability = mutability
        }
    );

    return true;
}

bool BindingContext::containsLocal(std::string_view name) const {
    return bindings_.contains(std::string(name));
}

bool BindingContext::contains(std::string_view name) const {
    return find(name) != nullptr;
}

const Binding* BindingContext::find(std::string_view name) const {
    const auto it = bindings_.find(std::string(name));
    if (it != bindings_.end()) {
        return &it->second;
    }

    return parent_ != nullptr ? parent_->find(name) : nullptr;
}

ExpressionValue BindingContext::resolve(std::string_view path) const {
    if (path.empty() || renderContext_ == nullptr) {
        return ExpressionValue();
    }

    const auto separator = path.find('.');
    const auto rootName = separator == std::string_view::npos
        ? path
        : path.substr(0, separator);

    if (const auto* binding = find(rootName)) {
        auto current = binding->value;
        if (separator == std::string_view::npos) {
            return current;
        }

        std::size_t start = separator + 1;
        while (start <= path.size()) {
            const auto end = path.find('.', start);
            const auto member = path.substr(
                start,
                end == std::string_view::npos
                    ? std::string_view::npos
                    : end - start
            );
            if (member.empty()) {
                return ExpressionValue();
            }
            current = current.member(member);
            if (current.isNull()) {
                return ExpressionValue();
            }
            if (end == std::string_view::npos) {
                break;
            }
            start = end + 1;
        }
        return current;
    }

    return resolveRenderContext(path, *renderContext_);
}

void BindingContext::materialize(RenderContext& target) const {
    if (parent_ != nullptr) {
        parent_->materialize(target);
    }

    for (const auto& [name, binding] : bindings_) {
        const auto& storage = binding.value.storage();

        if (const auto* boolean = std::get_if<bool>(&storage)) {
            target.set(name, *boolean);
            continue;
        }
        if (const auto* number = std::get_if<double>(&storage)) {
            if (std::isfinite(*number) && std::trunc(*number) == *number &&
                *number >= static_cast<double>(std::numeric_limits<int>::min()) &&
                *number <= static_cast<double>(std::numeric_limits<int>::max())
            ) {
                target.set(name, static_cast<int>(*number));
            } else {
                target.set(name, *number);
            }
            continue;
        }
        if (const auto* string = std::get_if<std::string>(&storage)) {
            target.set(name, *string);
            continue;
        }
        if (const auto* json = std::get_if<Json::Value>(&storage)) {
            target.set(name, *json);
            continue;
        }
        if (std::holds_alternative<std::monostate>(storage)) {
            target.set(name, Json::Value(Json::nullValue));
            continue;
        }

        // List/Range values have no legacy RenderContext representation.
        // Preserve their ExpressionValue form across component boundaries.
        target.set(name, binding.value);
    }
}

const RenderContext& BindingContext::renderContext() const {
    return *renderContext_;
}

} // namespace drogular::template_expression