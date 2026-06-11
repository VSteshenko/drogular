#pragma once

#include <drogular/component_registry.hpp>
#include <drogular/component.hpp>

#include <string>
#include <string_view>

namespace drogular::component_renderer {

/**
 * Renders self-closing component tags using the component registry.
 *
 * MVP supports:
 * <Card />
 */
std::string render(
    std::string_view html,
    const ComponentRegistry& registry,
    RenderContext& context
);

} // namespace drogular::component_renderer
