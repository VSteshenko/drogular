#pragma once

#include <drogular/page.hpp>

#include <string>
#include <cstring>

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
 * If the component HTML contains <slot/>, child HTML is inserted there.
 * Otherwise, child HTML is appended to the end.
 */
inline std::string renderComponentTree(
    Component& component,
    RenderContext& context
) {
    component.onInit(context);

    auto html = component.render(context);

    std::string childrenHtml;

    for (const auto& child : component.children()) {
        childrenHtml += renderComponentTree(*child, context);
    }

    constexpr auto slot = "<slot/>";

    const auto pos = html.find(slot);

    if (pos != std::string::npos) {
        html.replace(pos, std::strlen(slot), childrenHtml);
    } else {
        html += childrenHtml;
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

/**
 * Creates a page, runs its lifecycle with application services, and renders it.
 */
template <typename PageType>
RenderResult renderPage(ApplicationServices* services) {
    PageType page;
    RenderContext context;

    context.setServices(services);

    return {
        .html = renderComponentTree(page, context),
        .context = std::move(context)
    };
}

} // namespace drogular::test