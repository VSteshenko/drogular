#pragma once

#include "template_source.hpp"

namespace drogular::generation {

class EmbeddedTemplateSource final : public TemplateSource {
public:
    std::optional<std::string> load(std::string_view path) const override;
};

} // namespace drogular::generation