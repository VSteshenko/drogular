#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace drogular::generation {

class TemplateSource {
public:
    virtual ~TemplateSource() = default;

    virtual std::optional<std::string> load(std::string_view path) const = 0;
};

} // namespace drogular::generation