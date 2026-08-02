#pragma once

#include <drogular/component_registry.hpp>
#include <drogular/component.hpp>
#include <drogular/component_diagnostics.hpp>

#include <string>
#include <string_view>

namespace drogular::component_renderer {

struct RenderResult {
    std::string html;
    ComponentDiagnostics diagnostics;

    bool valid() const {
        return !diagnostics.hasErrors();
    }
};

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

/**
 * Renders component tags and reports component-related diagnostics.
 */
RenderResult renderWithDiagnostics(
    std::string_view html,
    const ComponentRegistry& registry,
    RenderContext& context,
    std::string sourceName = {}
);

} // namespace drogular::component_renderer