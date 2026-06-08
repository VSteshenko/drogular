#pragma once

#include <drogular/component.hpp>

#include <string>
#include <string_view>

namespace drogular::template_engine {

/**
 * Renders a template using values from the render context.
 *
 * MVP supports only string interpolation:
 *
 * {{ variable }}
 */
std::string render(
    std::string_view html,
    const RenderContext& context
);

} // namespace drogular::template_engine
