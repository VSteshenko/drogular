#include "embedded_template_source.hpp"

namespace drogular::generation {

std::optional<std::string> EmbeddedTemplateSource::load(std::string_view) const {
    return std::nullopt;
}

} // namespace drogular::generation