#pragma once

#include <string>

namespace drogular {

/**
 * Provides small reusable HTML snippets for PWA metadata.
 */
class PwaHtml {
public:
    /**
     * Creates a web app manifest link tag.
     */
    static std::string manifestLink(
        const std::string& path = "/assets/manifest.webmanifest"
    );

    /**
     * Creates a favicon link tag.
     */
    static std::string favicon(
        const std::string& path = "/assets/favicon.ico",
        const std::string& type = "image/x-icon"
    );

    /**
     * Creates a theme-color meta tag.
     */
    static std::string themeColor(
        const std::string& color
    );
};

} // namespace drogular