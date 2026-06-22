#pragma once

#include <drogular/static_file_mapping.hpp>

#include <string>
#include <vector>
#include <utility>
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

    void setTemplateCacheEnabled(bool enabled) {
        templateCacheEnabled_ = enabled;
    }

    bool templateCacheEnabled() const {
        return templateCacheEnabled_;
    }

    void addStaticFiles(
    std::string routePrefix,
    std::filesystem::path directory
) {
        staticFiles_.push_back({
            .routePrefix = std::move(routePrefix),
            .directory = std::move(directory)
        });
    }

    const std::vector<StaticFileMapping>& staticFiles() const {
        return staticFiles_;
    }

private:
    std::filesystem::path templateRoot_;
    bool templateCacheEnabled_ = true;
    std::vector<StaticFileMapping> staticFiles_;
};

} // namespace drogular
