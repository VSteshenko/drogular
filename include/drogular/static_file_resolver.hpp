#pragma once

#include <filesystem>
#include <optional>

namespace drogular {

/**
 * Resolves static file requests to filesystem paths.
 *
 * Applies path traversal protection and verifies that
 * requested files remain inside the configured root
 * directory.
 */
class StaticFileResolver {
public:
    /**
     * Creates a resolver for the specified root directory.
     */
    explicit StaticFileResolver(
        std::filesystem::path root
    );

    /**
     * Resolves a request path to a filesystem path.
     *
     * Returns std::nullopt when:
     *
     * - The file does not exist.
     * - The path escapes the configured root directory.
     * - The path does not reference a regular file.
     */
    std::optional<std::filesystem::path> resolve(
        const std::string& filePath
    ) const;

private:
    bool isInsideRoot(
        const std::filesystem::path& path
    ) const;

    std::filesystem::path root_;
};

} // namespace drogular