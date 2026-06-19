#include <drogular/template_loader.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace drogular {

TemplateLoader::TemplateLoader(
    std::filesystem::path root
)
    : root_(std::move(root)) {
}

const std::filesystem::path& TemplateLoader::root() const {
    return root_;
}

std::string TemplateLoader::load(
    const std::string& path
) const {
    std::filesystem::path resolvedPath(path);

    if (!root_.empty() &&
        resolvedPath.is_relative()) {
        resolvedPath = root_ / resolvedPath;
        }

    std::ifstream file(resolvedPath);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Template file not found: " +
            resolvedPath.string()
        );
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

} // namespace drogular