#pragma once

#include <drogular/pwa_options.hpp>
#include <drogular/component.hpp>

namespace drogular {

/**
 * Applies common PWA metadata and scripts to a page render context.
 */
class PwaPageSupport {
public:
    /**
     * Adds manifest, favicon, theme color and service worker
     * registration snippets to the render context.
     */
    static void apply(
        RenderContext& context,
        const PwaOptions& options = {}
    );
};

} // namespace drogular