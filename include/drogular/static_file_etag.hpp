#pragma once

#include <filesystem>
#include <string>

namespace drogular {

/**
 * Generates ETag values for static files.
 *
 * ETags are used by browsers and proxies to perform
 * conditional requests and reduce unnecessary file
 * transfers.
 */
class StaticFileEtag {
public:
    /**
     * Creates a weak file ETag based on file size and last write time.
     *
     * The generated value is suitable for static file caching and
     * conditional requests.
     */
    static std::string create(
        const std::filesystem::path& path
    );
};

} // namespace drogular
