#pragma once

#include <drogular/page.hpp>

#include <string>
#include <cstring>
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
    component.onInit(context);

    auto html = component.render(context);

    std::string defaultChildrenHtml;
    std::unordered_map<std::string, std::string> namedChildrenHtml;

    for (const auto& child : component.children()) {
        const auto childHtml = renderComponentTree(*child, context);

        if (child->slot().empty()) {
            defaultChildrenHtml += childHtml;
        } else {
            namedChildrenHtml[child->slot()] += childHtml;
        }
    }

    constexpr auto defaultSlot = "<slot/>";

    if (const auto pos = html.find(defaultSlot);
        pos != std::string::npos) {
        html.replace(pos, std::strlen(defaultSlot), defaultChildrenHtml);
        } else {
            html += defaultChildrenHtml;
        }

    for (const auto& [name, childHtml] : namedChildrenHtml) {
        const auto slot =
            "<slot name=\"" + name + "\"/>";

        size_t position = 0;

        while ((position = html.find(slot, position)) != std::string::npos) {
            html.replace(position, slot.size(), childHtml);
            position += childHtml.size();
        }
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