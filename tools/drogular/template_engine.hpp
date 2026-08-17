#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

namespace drogular::cli {

class TemplateEngine {
public:
    using Variables = std::unordered_map<std::string, std::string>;

    void renderFile(
        const std::filesystem::path& input,
        const std::filesystem::path& output,
        const Variables& variables) const;
};

}