#pragma once

#include <drogular/template/expression/value.hpp>

#include <string>
#include <string_view>
#include <unordered_map>

namespace drogular {

class RenderContext;

namespace template_expression {

enum class BindingMutability {
    Mutable,
    Constant
};

struct Binding {
    ExpressionValue value;
    BindingMutability mutability = BindingMutability::Mutable;
};

/**
 * Lexical expression bindings layered over an immutable RenderContext.
 *
 * Bindings are resolved from the nearest scope outward. If no lexical binding
 * matches the root identifier, lookup falls back to the base RenderContext.
 */
class BindingContext {
public:
    explicit BindingContext(const RenderContext& renderContext);
    explicit BindingContext(const BindingContext& parent);

    /** Creates a lexical child scope whose parent is this context. */
    [[nodiscard]] BindingContext createChild() const;

    /**
     * Defines a binding in the current scope.
     *
     * Returns false when the name already exists in this scope. Shadowing a
     * binding from a parent scope is allowed.
     */
    bool define(
        std::string name,
        ExpressionValue value,
        BindingMutability mutability = BindingMutability::Mutable
    );

    [[nodiscard]] bool containsLocal(std::string_view name) const;
    [[nodiscard]] bool contains(std::string_view name) const;

    /** Returns the nearest binding with this exact name, if any. */
    [[nodiscard]] const Binding* find(std::string_view name) const;

    /** Resolves a binding or RenderContext value, including dotted members. */
    [[nodiscard]] ExpressionValue resolve(std::string_view path) const;

    /**
     * Copies visible lexical bindings into a target RenderContext.
     *
     * This is intended for template/component boundaries that still consume a
     * RenderContext. The base fallback context is never mutated.
     */
    void materialize(RenderContext& target) const;

    /** Returns the immutable RenderContext used as the fallback data source. */
    [[nodiscard]] const RenderContext& renderContext() const;

private:
    const RenderContext* renderContext_ = nullptr;
    const BindingContext* parent_ = nullptr;
    std::unordered_map<std::string, Binding> bindings_;
};

} // namespace template_expression
} // namespace drogular