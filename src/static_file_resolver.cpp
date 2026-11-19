#include <drogular/static_file_resolver.hpp>

#include <algorithm>
#include <cctype>
#include <system_error>

namespace drogular {

StaticFileResolver::StaticFileResolver(
    std::filesystem::path root
)
    : root_(std::filesystem::weakly_canonical(root)) {
}

std::optional<std::filesystem::path> StaticFileResolver::resolve(
    const std::string& filePath,
    bool recursive
) const {
    // Do not interpret encoded path separators, dot-segments or NUL.
    // Reject these even if the HTTP stack has not decoded the URL yet.
    std::string lower = filePath;
    std::transform(lower.begin(), lower.end(), lower.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    for (const auto* encoded : {"%2e", "%2f", "%5c", "%00"}) {
        if (lower.find(encoded) != std::string::npos) {
            return std::nullopt;
        }
    }

    if (filePath.empty() || filePath.find('\0') != std::string::npos ||
        filePath.find('\\') != std::string::npos ||
        (!recursive && filePath.find('/') != std::string::npos)
    ) {
        return std::nullopt;
    }

    const std::filesystem::path relative(filePath);
    if (relative.is_absolute() || relative.has_root_path()) {
        return std::nullopt;
    }
    for (const auto& part : relative) {
        if (part == "." || part == "..") {
            return std::nullopt;
        }
    }

    std::error_code ec;
    const auto requested =
        std::filesystem::weakly_canonical(root_ / relative, ec);
    if (ec || !isInsideRoot(requested)) {
        return std::nullopt;
    }
    if (!std::filesystem::is_regular_file(requested, ec) || ec) {
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