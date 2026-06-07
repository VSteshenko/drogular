#pragma once

#include <drogular/page.hpp>

#include <string>

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
 */
inline std::string renderComponentTree(
    Component& component,
    RenderContext& context
) {
    component.onInit(context);

    std::string html = component.render(context);

    for (const auto& child : component.children()) {
        html += renderComponentTree(*child, context);
    }

    return html;
}

/**
 * Creates a page, runs its lifecycle, and renders it.
 */
template <typename PageType>
RenderResult renderPage() {
    PageType page;
    RenderContext context;

    return {
        .html = renderComponentTree(page, context),
        .context = std::move(context)
    };
}

} // namespace drogular::test