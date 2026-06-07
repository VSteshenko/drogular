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
 * Creates a page, runs its lifecycle, and renders it.
 */
template <typename PageType>
RenderResult renderPage() {
    PageType page;
    RenderContext context;

    page.onInit(context);

    return {
        .html = page.render(context),
        .context = std::move(context)
    };
}

} // namespace drogular::test