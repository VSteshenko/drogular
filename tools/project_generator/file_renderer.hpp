#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace drogular::generation {

class FileRenderer {
public:
    using Variables = std::unordered_map<std::string, std::string>;

    [[nodiscard]] std::string render(
        std::string_view content,
        const Variables& variables) const;
};

} // namespace drogular::generation