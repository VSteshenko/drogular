#pragma once

#include <drogular/page.hpp>
#include <drogular/render_context.hpp>
#include <drogular/component_renderer.hpp>

#include <drogon/HttpRequest.h>

#include <string>
#include <unordered_map>

namespace drogular::test {

/**
 * Stores the result of rendering a page in tests.
 */
struct RenderResult {
    std::string html;
    RenderContext context;
};

/**
 * Returns true if the text contains the specified substring.
 */
inline bool contains(
    const std::string& text,
    const std::string& value
) {
    return text.find(value) != std::string::npos;
}

/**
 * Renders a component and its children.
 *
 * Supports:
 * - <slot/> for default children
 * - <slot name="header"/> for named children
 */
inline std::string renderComponentTree(
    Component& component,
    RenderContext& context
) {
    return component_renderer::renderComponentTree(component, context);
}

/**
 * Creates a page, runs its lifecycle with application services, and renders it.
 */
template <typename PageType>
RenderResult renderPage(
    ApplicationServices* services = nullptr,
    const drogon::HttpRequestPtr& request = nullptr
) {
    PageType page;

    RenderContext context;

    if (services != nullptr) {
        context.setServices(services);
    }

    if (request != nullptr) {
        context.setRequest(request);
    }

    return {
        .html = component_renderer::renderComponentTree(page, context),
        .context = std::move(context)
    };
}

} // namespace drogular::test