#pragma once

#include <drogular/component.hpp>

#include <string>

namespace drogular {

/**
 * Applies common page-level render context values.
 *
 * PageSupport provides reusable values that are useful for
 * regular pages, layouts and PWA-enabled applications.
 */
class PageSupport {
public:
    /**
     * Adds common page variables to the render context.
     *
     * Added values:
     *
     * - pageTitle
     * - currentPath
     * - manifestLink
     * - faviconLink
     * - themeColorMeta
     * - serviceWorkerRegistration
     */
    static void apply(
        RenderContext& context,
        const std::string& pageTitle = ""
    );
};

} // namespace drogular