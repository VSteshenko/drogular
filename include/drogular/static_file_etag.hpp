#pragma once

#include <filesystem>
#include <string>

namespace drogular {

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
