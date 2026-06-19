#pragma once

#include <filesystem>

namespace drogular {

/**
 * Stores application-level configuration.
 */
class ApplicationOptions {
public:
    void setTemplateRoot(
        std::filesystem::path root
    ) {
        templateRoot_ = std::move(root);
    }

    const std::filesystem::path& templateRoot() const {
        return templateRoot_;
    }

private:
    std::filesystem::path templateRoot_;
};

} // namespace drogular
