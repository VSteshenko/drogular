#pragma once

#include <drogular/render_context.hpp>
#include <drogular/template/expression/binding_context.hpp>
#include <drogular/template/expression/functions.hpp>

#include <span>

inline drogular::template_expression::ExpressionFunctionCallback
portalTranslationExpressionFunction() {
    return [](
        std::span<const drogular::template_expression::ExpressionValue> arguments,
        const drogular::template_expression::BindingContext& bindings
    ) -> drogular::template_expression::ExpressionValue {
        if (arguments.size() != 1) {
            return {};
        }

        const auto key = arguments.front().string();
        if (!key.has_value()) {
            return {};
        }

        auto& context = const_cast<drogular::RenderContext&>(
            bindings.renderContext()
        );

        return drogular::template_expression::ExpressionValue(
            context.translate(*key)
        );
    };
}