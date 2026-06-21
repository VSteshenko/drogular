#pragma once

#include <drogular/page.hpp>

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
    component.onInit(context);

    auto html = component.render(context);

    std::string defaultChildrenHtml;

    if (const auto slotHtml =
    component.param<std::string>("__slot")) {
        defaultChildrenHtml += *slotHtml;
    }

    std::unordered_map<std::string, std::string> namedChildrenHtml;

    for (const auto& child : component.children()) {
        auto childContext = context.createChild();

        const auto childHtml =
            renderComponentTree(*child, childContext);

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

    component.onDestroy(context);

    return html;
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

    page.onInit(context);

    return {
        .html = renderComponentTree(page, context),
        .context = std::move(context)
    };
}

} // namespace drogular::test