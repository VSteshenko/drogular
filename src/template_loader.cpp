#include <drogular/template_loader.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace drogular {

std::string TemplateLoader::load(
    const std::string& path
) const {
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Template file not found: " + path
        );
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

} // namespace drogular