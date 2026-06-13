#pragma once

#include <drogular/component.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace drogular::template_compiler {

/**
 * Resolves a template expression into escaped HTML text.
 */
std::optional<std::string> resolveVariable(
    std::string_view expression,
    const RenderContext& context
);

/**
 * Resolves a template expression into raw text.
 */
std::optional<std::string> resolveRawVariable(
    std::string_view expression,
    const RenderContext& context
);

/**
 * Evaluates a template condition.
 */
bool evaluateCondition(
    std::string_view expression,
    const RenderContext& context
);

} // namespace drogular::template_compiler
