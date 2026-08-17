#include "template_engine.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace drogular::cli {

void TemplateEngine::renderFile(
    const std::filesystem::path& input,
    const std::filesystem::path& output,
    const Variables& variables) const
{
    std::ifstream source(input, std::ios::binary);
    if (!source) {
        throw std::runtime_error("Cannot open template: " + input.string());
    }

    std::ostringstream buffer;
    buffer << source.rdbuf();
    std::string content = buffer.str();

    for (const auto& [name, value] : variables) {
        const std::string placeholder = "@" + name + "@";
        std::size_t position = 0;

        while ((position = content.find(placeholder, position)) != std::string::npos) {
            content.replace(position, placeholder.size(), value);
            position += value.size();
        }
    }

    if (const auto parent = output.parent_path(); !parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    std::ofstream destination(output, std::ios::binary);
    if (!destination) {
        throw std::runtime_error("Cannot create file: " + output.string());
    }

    destination << content;
}

}