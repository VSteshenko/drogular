#pragma once

#include <filesystem>
#include <optional>

namespace drogular {

class StaticFileResolver {
public:
    explicit StaticFileResolver(
        std::filesystem::path root
    );

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