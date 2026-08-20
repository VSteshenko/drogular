#pragma once

#include "file_renderer.hpp"

namespace drogular::generation {

class TemplateRegistry;
class TemplateSource;

class ProjectGenerator {
public:
    ProjectGenerator(
        const TemplateRegistry& registry,
        const TemplateSource& source,
        FileRenderer renderer = {});

private:
    const TemplateRegistry& registry_;
    const TemplateSource& source_;
    FileRenderer renderer_;
};

} // namespace drogular::generation