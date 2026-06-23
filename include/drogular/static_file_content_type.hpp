#pragma once

#include <filesystem>
#include <string>

namespace drogular {

/**
 * Detects MIME content types for static files.
 *
 * Used when serving static files and file responses to
 * automatically set the HTTP Content-Type header.
 */
class StaticFileContentType {
public:
    /**
     * Returns the MIME content type associated with
     * the file extension.
     *
     * Unknown extensions are returned as
     * application/octet-stream.
     */
    static std::string fromPath(
        const std::filesystem::path& path
    );
};

} // namespace drogular