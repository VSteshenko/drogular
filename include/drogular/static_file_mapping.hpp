#pragma once

#include <filesystem>
#include <string>

namespace drogular {

/// Maps a URL prefix to a filesystem directory.
///
/// Example:
///
/// routePrefix = "/assets"
/// directory   = "public"
///
/// Request:
///
/// /assets/logo.png
///
/// File:
///
/// public/logo.png
struct StaticFileMapping {
    std::string routePrefix;
    std::filesystem::path directory;
};

} // namespace drogular