#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace drogular::generation {

struct TemplateFile {
    std::string source;
    std::filesystem::path destination;
    bool executable{false};
};

struct ProjectTemplate {
    std::string id;
    std::string name;
    std::string description;
    std::vector<TemplateFile> files;
};

} // namespace drogular::generation