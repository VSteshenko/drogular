#include "project_generator.hpp"

#include "project_template.hpp"
#include "template_registry.hpp"
#include "template_source.hpp"

#include <fstream>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;

namespace drogular::generation {
namespace {

fs::path outputPath(
    const TemplateFile& file,
    const FileRenderer& renderer,
    const FileRenderer::Variables& variables)
{
    const fs::path relative{
        renderer.render(file.destination.generic_string(), variables)};

    if (relative.empty() || relative.is_absolute()) {
        throw std::runtime_error(
            "Project template contains an invalid destination path: " +
            file.destination.generic_string());
    }

    for (const auto& part : relative) {
        if (part == "..") {
            throw std::runtime_error(
                "Project template destination escapes the project directory: " +
                relative.generic_string());
        }
    }

    return relative;
}

void writeFile(const fs::path& path, std::string_view content) {
    if (const fs::path parent = path.parent_path(); !parent.empty()) {
        fs::create_directories(parent);
    }

    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) {
        throw std::runtime_error("Unable to create generated file: " + path.string());
    }

    stream.write(content.data(), static_cast<std::streamsize>(content.size()));
    if (!stream) {
        throw std::runtime_error("Unable to write generated file: " + path.string());
    }
}

void makeExecutable(const fs::path& path) {
    fs::permissions(
        path,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add);
}

} // namespace

ProjectGenerator::ProjectGenerator(
    const TemplateRegistry& registry,
    const TemplateSource& source,
    FileRenderer renderer)
    : registry_(registry),
      source_(source),
      renderer_(std::move(renderer))
{
}

void ProjectGenerator::generate(const GenerationRequest& request) const {
    if (request.templateId.empty()) {
        throw std::invalid_argument("Project template id must not be empty.");
    }
    if (request.destination.empty()) {
        throw std::invalid_argument("Project destination must not be empty.");
    }

    const ProjectTemplate* projectTemplate = registry_.find(request.templateId);
    if (projectTemplate == nullptr) {
        throw std::runtime_error(
            "Project template is not registered: " + request.templateId);
    }

    if (fs::exists(request.destination)) {
        throw std::runtime_error(
            "Project destination already exists: " + request.destination.string());
    }

    try {
        fs::create_directories(request.destination);

        for (const TemplateFile& file : projectTemplate->files) {
            const auto content = source_.load(file.source);
            if (!content) {
                throw std::runtime_error(
                    "Project template file not found: " + file.source);
            }

            const fs::path destination = request.destination /
                outputPath(file, renderer_, request.variables);

            writeFile(destination, renderer_.render(*content, request.variables));

            if (file.executable) {
                makeExecutable(destination);
            }
        }
    } catch (...) {
        std::error_code ignored;
        fs::remove_all(request.destination, ignored);
        throw;
    }
}

} // namespace drogular::generation