#include "embedded_template_source.hpp"
#include "project_generator.hpp"
#include "template_registry.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#ifndef DROGULAR_CLI_VERSION
#define DROGULAR_CLI_VERSION "development"
#endif

#ifndef DROGULAR_CLI_GIT_REF
#define DROGULAR_CLI_GIT_REF "main"
#endif

namespace fs = std::filesystem;

namespace {

void printHelp() {
    std::cout << "Drogular CLI\n\n"
              << "Usage:\n"
              << "  drogular new <project>\n\n"
              << "Options:\n"
              << "  --help       Show this help\n"
              << "  --version    Show version\n";
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

std::string projectNamespace(const std::string& projectName) {
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

std::string currentYear() {
    const auto today = std::chrono::floor<std::chrono::days>(
        std::chrono::system_clock::now());
    const std::chrono::year_month_day date{today};
    return std::to_string(static_cast<int>(date.year()));
}

int createProject(const std::string& name) {
    if (!validProjectName(name)) {
        std::cerr << "Invalid project name: " << name << '\n';
        std::cerr << "Use only letters, digits, '-' and '_'.\n";
        return 1;
    }

    try {
        drogular::generation::EmbeddedTemplateSource source;
        drogular::generation::TemplateRegistry registry;
        registry.load(source);

        drogular::generation::ProjectGenerator generator(registry, source);
        generator.generate({
            .templateId = "minimal",
            .destination = fs::path{name},
            .variables = {
                {"PROJECT_NAME", name},
                {"PROJECT_NAMESPACE", projectNamespace(name)},
                {"DROGULAR_VERSION", DROGULAR_CLI_VERSION},
                {"DROGULAR_GIT_REF", DROGULAR_CLI_GIT_REF},
                {"YEAR", currentYear()},
            },
        });
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    std::cout << "Created Drogular project: " << name << "\n\n"
              << "Next steps:\n"
              << "  cd " << name << '\n'
              << "  cmake -S . -B build\n"
              << "  cmake --build build\n"
              << "  ./build/" << name << '\n';

    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc == 2) {
        const std::string_view argument{argv[1]};

        if (argument == "--help" || argument == "-h") {
            printHelp();
            return 0;
        }

        if (argument == "--version" || argument == "-v") {
            std::cout << "Drogular CLI " << DROGULAR_CLI_VERSION << '\n';
            return 0;
        }
    }

    if (argc == 3 && std::string_view(argv[1]) == "new") {
        return createProject(argv[2]);
    }

    printHelp();
    return argc == 1 ? 0 : 1;
}