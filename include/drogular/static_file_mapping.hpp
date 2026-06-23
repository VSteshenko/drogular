#pragma once

#include <filesystem>
#include <string>

namespace drogular {

/**
 * Maps a URL route prefix to a filesystem directory.
 *
 * Example:
 *
 * routePrefix = "/assets"
 * directory   = "public"
 *
 * Request:
 *
 * /assets/logo.svg
 *
 * File:
 *
 * public/logo.svg
 */
struct StaticFileMapping {
    /**
     * URL route prefix.
     */
    std::string routePrefix;

    /**
     * Filesystem directory serving requests
     * under the route prefix.
     */
    std::filesystem::path directory;
};

} // namespace drogular