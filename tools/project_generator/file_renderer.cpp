#include "file_renderer.hpp"

namespace drogular::generation {

std::string FileRenderer::render(
    std::string_view content,
    const Variables& variables) const
{
    std::string result(content);

    for (const auto& [name, value] : variables) {
        const std::string placeholder = "{{" + name + "}}";
        std::size_t position = 0;

        while ((position = result.find(placeholder, position)) != std::string::npos) {
            result.replace(position, placeholder.size(), value);
            position += value.size();
        }
    }

    return result;
}

} // namespace drogular::generation