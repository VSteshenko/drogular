#pragma once

#include <filesystem>
#include <string>

namespace drogular::cli {

class ProjectGenerator {
public:
    ProjectGenerator(
        std::filesystem::path templatesRoot,
        std::string drogularVersion);

    void generate(const std::string& projectName) const;

private:
    std::filesystem::path templatesRoot_;
    std::string drogularVersion_;
};

}