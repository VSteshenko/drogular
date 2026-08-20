#pragma once

#include "project_template.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace drogular::generation {

class TemplateSource {
public:
    virtual ~TemplateSource() = default;

    virtual std::optional<std::string> load(std::string_view path) const = 0;
    [[nodiscard]] virtual std::vector<ProjectTemplate> projectTemplates() const {
        return {};
    }
};

} // namespace drogular::generation