#pragma once

#include "project_template.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace drogular::generation::detail {

const std::unordered_map<std::string, std::string>& embeddedTemplateFiles();
const std::vector<ProjectTemplate>& embeddedProjectTemplates();

} // namespace drogular::generation::detail
