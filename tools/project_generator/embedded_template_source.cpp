#include "embedded_template_source.hpp"
#include "embedded_template_data.hpp"

namespace drogular::generation {

std::optional<std::string> EmbeddedTemplateSource::load(std::string_view path) const {
    const auto& files = detail::embeddedTemplateFiles();
    const auto it = files.find(std::string(path));
    return it == files.end() ? std::nullopt : std::optional{it->second};
}

std::vector<ProjectTemplate> EmbeddedTemplateSource::projectTemplates() const {
    const auto& templates = detail::embeddedProjectTemplates();
    return {templates.begin(), templates.end()};
}

} // namespace drogular::generation