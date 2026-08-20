#include "project_generator.hpp"

#include "template_registry.hpp"
#include "template_source.hpp"

#include <utility>

namespace drogular::generation {

ProjectGenerator::ProjectGenerator(
    const TemplateRegistry& registry,
    const TemplateSource& source,
    FileRenderer renderer)
    : registry_(registry),
      source_(source),
      renderer_(std::move(renderer))
{
}

} // namespace drogular::generation