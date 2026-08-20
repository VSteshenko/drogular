#pragma once

#include "project_template.hpp"

#include <string_view>
#include <unordered_map>
#include <vector>

namespace drogular::generation {

class TemplateSource;

class TemplateRegistry {
public:
    void add(ProjectTemplate projectTemplate);
    void load(const TemplateSource& source);

    [[nodiscard]] const ProjectTemplate* find(std::string_view id) const;
    [[nodiscard]] std::vector<const ProjectTemplate*> templates() const;

private:
    std::unordered_map<std::string, ProjectTemplate> templates_;
};

} // namespace drogular::generation