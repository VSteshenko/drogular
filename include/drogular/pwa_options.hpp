#pragma once

#include <string>

namespace drogular {

/**
 * Stores PWA-related page metadata options.
 */
struct PwaOptions {
    /**
     * Web app manifest URL.
     */
    std::string manifestPath =
        "/assets/manifest.webmanifest";

    /**
     * Favicon URL.
     */
    std::string faviconPath =
        "/assets/favicon.ico";

    /**
     * Favicon MIME type.
     */
    std::string faviconType =
        "image/x-icon";

    /**
     * Browser theme color.
     */
    std::string themeColor =
        "#4f46e5";

    /**
     * Service worker registration URL.
     */
    std::string serviceWorkerPath =
        "/service-worker.js";
};

} // namespace drogular