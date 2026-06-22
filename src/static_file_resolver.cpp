#include <drogular/static_file_resolver.hpp>

namespace drogular {

StaticFileResolver::StaticFileResolver(
    std::filesystem::path root
)
    : root_(std::filesystem::weakly_canonical(root)) {
}

std::optional<std::filesystem::path> StaticFileResolver::resolve(
    const std::string& filePath
) const {
    const auto requested =
        std::filesystem::weakly_canonical(
            root_ / filePath
        );

    if (!isInsideRoot(requested)) {
        return std::nullopt;
    }

    if (!std::filesystem::exists(requested) ||
        !std::filesystem::is_regular_file(requested)) {
        return std::nullopt;
    }

    return requested;
}

bool StaticFileResolver::isInsideRoot(
    const std::filesystem::path& path
) const {
    const auto rootString =
        root_.string();

    const auto pathString =
        path.string();

    const auto prefix =
        rootString +
        std::string(
            1,
            std::filesystem::path::preferred_separator
        );

    return pathString == rootString ||
           pathString.rfind(prefix, 0) == 0;
}

} // namespace drogular