#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace drogular::cli {

enum class Command {
    Help,
    Version,
    NewProject,
    ListTemplates,
    Invalid,
};

struct Options {
    Command command{Command::Help};
    std::string projectPath;
    std::string projectName;
    std::string templateId{"minimal"};
    std::string error;
};

[[nodiscard]] Options parseArguments(const std::vector<std::string_view>& arguments);
[[nodiscard]] bool validProjectName(std::string_view name);
[[nodiscard]] std::string projectNamespace(std::string_view projectName);

} // namespace drogular::cli