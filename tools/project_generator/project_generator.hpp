#pragma once

#include "file_renderer.hpp"

#include <filesystem>
#include <string>

namespace drogular::generation {

class TemplateRegistry;
class TemplateSource;

struct GenerationRequest {
    std::string templateId;
    std::filesystem::path destination;
    FileRenderer::Variables variables;
};

class ProjectGenerator {
public:
    ProjectGenerator(
        const TemplateRegistry& registry,
        const TemplateSource& source,
        FileRenderer renderer = {});

    void generate(const GenerationRequest& request) const;

private:
    const TemplateRegistry& registry_;
    const TemplateSource& source_;
    FileRenderer renderer_;
};

} // namespace drogular::generation