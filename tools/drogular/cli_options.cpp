#include "cli_options.hpp"

#include <filesystem>

namespace fs = std::filesystem;

namespace drogular::cli {
namespace {

Options invalid(std::string message) {
    Options result;
    result.command = Command::Invalid;
    result.error = std::move(message);
    return result;
}

} // namespace

Options parseArguments(const std::vector<std::string_view>& arguments) {
    if (arguments.empty()) {
        return {};
    }

    if (arguments.size() == 1) {
        if (arguments[0] == "--help" || arguments[0] == "-h") {
            Options result;
            result.command = Command::Help;
            return result;
        }
        if (arguments[0] == "--version" || arguments[0] == "-v") {
            Options result;
            result.command = Command::Version;
            return result;
        }
        if (arguments[0] == "templates") {
            Options result;
            result.command = Command::ListTemplates;
            return result;
        }
    }

    if (arguments[0] != "new") {
        return invalid("Unknown command: " + std::string(arguments[0]));
    }

    if (arguments.size() < 2) {
        return invalid("Missing project name.");
    }

    const fs::path requestedPath{arguments[1]};
    fs::path normalizedPath = requestedPath.lexically_normal();
    while (!normalizedPath.empty() && normalizedPath.filename().empty()) {
        const fs::path parent = normalizedPath.parent_path();
        if (parent == normalizedPath) {
            break;
        }
        normalizedPath = parent;
    }

    const std::string projectName = normalizedPath.filename().string();
    if (!validProjectName(projectName)) {
        return invalid(
            "Invalid project name: " + projectName +
            ". Use only letters, digits, '-' and '_'.");
    }

    Options result;
    result.command = Command::NewProject;
    result.projectPath = normalizedPath.string();
    result.projectName = projectName;

    bool templateSpecified = false;
    for (std::size_t index = 2; index < arguments.size(); ++index) {
        const std::string_view argument = arguments[index];

        if (argument == "--template") {
            if (templateSpecified) {
                return invalid("Project template was specified more than once.");
            }
            if (++index >= arguments.size()) {
                return invalid("Missing value for --template.");
            }
            if (arguments[index].empty()) {
                return invalid("Project template id must not be empty.");
            }

            result.templateId = std::string(arguments[index]);
            templateSpecified = true;
            continue;
        }

        constexpr std::string_view prefix = "--template=";
        if (argument.starts_with(prefix)) {
            if (templateSpecified) {
                return invalid("Project template was specified more than once.");
            }

            const std::string_view id = argument.substr(prefix.size());
            if (id.empty()) {
                return invalid("Project template id must not be empty.");
            }

            result.templateId = std::string(id);
            templateSpecified = true;
            continue;
        }

        return invalid("Unknown option: " + std::string(argument));
    }

    return result;
}

bool validProjectName(std::string_view name) {
    if (name.empty()) {
        return false;
    }

    for (const char ch : name) {
        const bool valid =
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '_' || ch == '-';

        if (!valid) {
            return false;
        }
    }

    return true;
}

std::string projectNamespace(std::string_view projectName) {
    std::string result;
    result.reserve(projectName.size() + 1);

    if (!projectName.empty() && projectName.front() >= '0' && projectName.front() <= '9') {
        result.push_back('_');
    }

    for (const char ch : projectName) {
        result.push_back(ch == '-' ? '_' : ch);
    }

    return result;
}

} // namespace drogular::cli