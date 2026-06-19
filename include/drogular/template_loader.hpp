#pragma once

#include <filesystem>
#include <string>

namespace drogular {

class TemplateLoader {
public:
    TemplateLoader() = default;

    explicit TemplateLoader(
        std::filesystem::path root
    );

    std::string load(
        const std::string& path
    ) const;

    const std::filesystem::path& root() const;

private:
    std::filesystem::path root_;
};

} // namespace drogular