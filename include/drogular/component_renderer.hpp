#pragma once

#include <drogular/component_registry.hpp>
#include <drogular/component.hpp>
#include <drogular/component_diagnostics.hpp>

#include <string>
#include <string_view>

namespace drogular::template_expression {
    class BindingContext;
}

namespace drogular::component_renderer {

struct RenderResult {
    std::string html;
    ComponentDiagnostics diagnostics;

    bool valid() const {
        return !diagnostics.hasErrors();
    }
};

/**
 * Runs the complete lifecycle for a component tree.
 *
 * Calls onInit(), render(), renders child slots recursively, and
 * guarantees onDestroy() after rendering.
 */
std::string renderComponentTree(
    Component& component,
    RenderContext& context
);

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
 * Renders component tags while preserving an existing lexical binding scope
 * for component attribute expressions.
 *
 * Component instances still receive a RenderContext; lexical bindings needed
 * by the component body are bridged explicitly at that boundary.
 */
std::string render(
    std::string_view html,
    const ComponentRegistry& registry,
    RenderContext& context,
    const template_expression::BindingContext& bindings
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