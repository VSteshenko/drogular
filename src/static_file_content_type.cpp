#include <drogular/static_file_content_type.hpp>

#include <algorithm>

namespace drogular {

std::string StaticFileContentType::fromPath(
    const std::filesystem::path& path
) {
    auto extension =
        path.extension().string();

    std::transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        [](unsigned char character) {
            return static_cast<char>(
                std::tolower(character)
            );
        }
    );

    if (extension == ".html") return "text/html";
    if (extension == ".css") return "text/css";
    if (extension == ".js") return "application/javascript";
    if (extension == ".json") return "application/json";
    if (extension == ".webmanifest") return "application/manifest+json";

    if (extension == ".svg") return "image/svg+xml";
    if (extension == ".png") return "image/png";
    if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    if (extension == ".ico") return "image/x-icon";
    if (extension == ".webp") return "image/webp";

    if (extension == ".txt") return "text/plain";
    if (extension == ".pdf") return "application/pdf";

    return "application/octet-stream";
}

} // namespace drogular