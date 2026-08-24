#include "cli_options.hpp"
#include "embedded_template_source.hpp"
#include "project_generator.hpp"
#include "template_registry.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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
              << "  drogular new <path/to/project> [--template <id>]\n"
              << "  drogular templates\n\n"
              << "Options:\n"
              << "  --template <id>  Project template to use (default: minimal)\n"
              << "  --help           Show this help\n"
              << "  --version        Show version\n";
}

std::string currentYear() {
    const auto today = std::chrono::floor<std::chrono::days>(
        std::chrono::system_clock::now());
    const std::chrono::year_month_day date{today};
    return std::to_string(static_cast<int>(date.year()));
}

void loadTemplates(
    drogular::generation::EmbeddedTemplateSource& source,
    drogular::generation::TemplateRegistry& registry
) {
    registry.load(source);
}

int listTemplates() {
    try {
        drogular::generation::EmbeddedTemplateSource source;
        drogular::generation::TemplateRegistry registry;
        loadTemplates(source, registry);

        auto templates = registry.templates();
        std::sort(
            templates.begin(),
            templates.end(),
            [](const auto* lhs, const auto* rhs) { return lhs->id < rhs->id; }
        );

        std::cout << "Available project templates:\n";
        for (const auto* projectTemplate : templates) {
            std::cout << "  " << projectTemplate->id;
            if (!projectTemplate->description.empty()) {
                std::cout << "  " << projectTemplate->description;
            }
            std::cout << '\n';
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

int createProject(
    const std::string& path,
    const std::string& name,
    const std::string& templateId
) {
    try {
        drogular::generation::EmbeddedTemplateSource source;
        drogular::generation::TemplateRegistry registry;
        loadTemplates(source, registry);

        if (registry.find(templateId) == nullptr) {
            std::cerr << "Unknown project template: " << templateId << '\n';
            std::cerr << "Run 'drogular templates' to list available templates.\n";

            return 1;
        }

        drogular::generation::ProjectGenerator generator(registry, source);
        generator.generate({
            .templateId = templateId,
            .destination = fs::path{path},
            .variables = {
                {"PROJECT_NAME", name},
                {"PROJECT_NAMESPACE", drogular::cli::projectNamespace(name)},
                {"DROGULAR_VERSION", DROGULAR_CLI_VERSION},
                {"DROGULAR_GIT_REF", DROGULAR_CLI_GIT_REF},
                {"YEAR", currentYear()},
            },
        });
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    std::cout << "Created Drogular project: " << name
              << " (template: " << templateId << ")\n\n"
              << "Next steps:\n"
              << "  cd " << path << '\n'
              << "  cmake -S . -B build\n"
              << "  cmake --build build\n"
              << "  ./build/" << name << '\n';

    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    std::vector<std::string_view> arguments;
    arguments.reserve(argc > 1 ? static_cast<std::size_t>(argc - 1) : 0);
    for (int index = 1; index < argc; ++index) {
        arguments.emplace_back(argv[index]);
    }

    const drogular::cli::Options options = drogular::cli::parseArguments(arguments);
    switch (options.command) {
    case drogular::cli::Command::Help:
        printHelp();
        return 0;

    case drogular::cli::Command::Version:
        std::cout << "Drogular CLI " << DROGULAR_CLI_VERSION << '\n';
        return 0;

    case drogular::cli::Command::ListTemplates:
        return listTemplates();

    case drogular::cli::Command::NewProject:
        return createProject(options.projectPath, options.projectName, options.templateId);

    case drogular::cli::Command::Invalid:
        if (!options.error.empty()) {
            std::cerr << options.error << "\n\n";
        }
        printHelp();
        return 1;
    }

    return 1;
}