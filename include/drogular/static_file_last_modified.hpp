#pragma once

#include <filesystem>
#include <string>

namespace drogular {

/**
 * Generates Last-Modified header values for static files.
 *
 * Last-Modified is used by browsers and proxies to perform
 * conditional requests based on file modification time.
 */
class StaticFileLastModified {
public:
    /**
     * Creates an HTTP date string from the file last write time.
     */
    static std::string create(
        const std::filesystem::path& path
    );

    /**
     * Returns true when the request If-Modified-Since value
     * matches the current file Last-Modified value.
     */
    static bool matches(
        const std::filesystem::path& path,
        const std::string& requestValue
    );
};

} // namespace drogular