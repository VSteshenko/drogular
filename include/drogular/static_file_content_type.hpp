#pragma once

#include <filesystem>
#include <string>

namespace drogular {

class StaticFileContentType {
public:
    /**
     * Detects content type from file extension.
     *
     * Unknown extensions are returned as
     * application/octet-stream.
     */
    static std::string fromPath(
        const std::filesystem::path& path
    );
};

} // namespace drogular