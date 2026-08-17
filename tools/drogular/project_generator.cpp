#include "project_generator.hpp"

#include "template_engine.hpp"

#include <chrono>
#include <stdexcept>
#include <utility>

namespace fs = std::filesystem;

namespace drogular::cli {
namespace {

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

}

ProjectGenerator::ProjectGenerator(
    fs::path templatesRoot,
    std::string drogularVersion)
    : templatesRoot_(std::move(templatesRoot)),
      drogularVersion_(std::move(drogularVersion))
{
}

void ProjectGenerator::generate(const std::string& projectName) const {
    const fs::path sourceRoot = templatesRoot_ / "minimal";
    if (!fs::is_directory(sourceRoot)) {
        throw std::runtime_error(
            "Drogular project template not found: " + sourceRoot.string());
    }

    const fs::path destinationRoot{projectName};
    if (fs::exists(destinationRoot)) {
        throw std::runtime_error(
            "Path already exists: " + destinationRoot.string());
    }

    const TemplateEngine::Variables variables{
        {"PROJECT_NAME", projectName},
        {"PROJECT_NAMESPACE", projectNamespace(projectName)},
        {"DROGULAR_VERSION", drogularVersion_},
        {"YEAR", currentYear()},
    };

    TemplateEngine engine;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(sourceRoot)) {
            const fs::path relative = fs::relative(entry.path(), sourceRoot);

            if (entry.is_directory()) {
                fs::create_directories(destinationRoot / relative);
                continue;
            }

            if (!entry.is_regular_file() || relative == "template.json") {
                continue;
            }

            fs::path outputRelative = relative;
            if (outputRelative.extension() == ".in") {
                outputRelative.replace_extension();
                engine.renderFile(
                    entry.path(),
                    destinationRoot / outputRelative,
                    variables);
            } else {
                const fs::path output = destinationRoot / outputRelative;
                if (const auto parent = output.parent_path(); !parent.empty()) {
                    fs::create_directories(parent);
                }
                fs::copy_file(entry.path(), output);
            }
        }
    } catch (...) {
        std::error_code ignored;
        fs::remove_all(destinationRoot, ignored);
        throw;
    }
}

}