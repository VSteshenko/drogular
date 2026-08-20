#include "template_registry.hpp"
#include "template_source.hpp"

#include <stdexcept>
#include <utility>

namespace drogular::generation {

void TemplateRegistry::load(const TemplateSource& source) {
    for (ProjectTemplate projectTemplate : source.projectTemplates()) {
        add(std::move(projectTemplate));
    }
}

void TemplateRegistry::add(ProjectTemplate projectTemplate) {
    if (projectTemplate.id.empty()) {
        throw std::invalid_argument("Project template id must not be empty.");
    }

    const std::string id = projectTemplate.id;
    const auto [_, inserted] = templates_.emplace(id, std::move(projectTemplate));
    if (!inserted) {
        throw std::invalid_argument("Project template is already registered: " + id);
    }
}

const ProjectTemplate* TemplateRegistry::find(std::string_view id) const {
    const auto it = templates_.find(std::string(id));
    return it == templates_.end() ? nullptr : &it->second;
}

std::vector<const ProjectTemplate*> TemplateRegistry::templates() const {
    std::vector<const ProjectTemplate*> result;
    result.reserve(templates_.size());

    for (const auto& [_, projectTemplate] : templates_) {
        result.push_back(&projectTemplate);
    }

    return result;
}

} // namespace drogular::generation