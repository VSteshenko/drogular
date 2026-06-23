#pragma once

#include <drogular/static_file_mapping.hpp>

#include <string>
#include <vector>
#include <utility>
#include <filesystem>
#include <chrono>

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

    /**
     * Enables or disables cache headers for static file responses.
     */
    void setStaticFileCacheEnabled(bool enabled) {
        staticFileCacheEnabled_ = enabled;
    }

    /**
     * Returns whether static file cache headers are enabled.
     */
    bool staticFileCacheEnabled() const {
        return staticFileCacheEnabled_;
    }

    /**
     * Sets max-age for static file Cache-Control headers.
     */
    void setStaticFileCacheMaxAge(
        std::chrono::seconds maxAge
    ) {
        staticFileCacheMaxAge_ = maxAge;
    }

    /**
     * Returns max-age for static file Cache-Control headers.
     */
    std::chrono::seconds staticFileCacheMaxAge() const {
        return staticFileCacheMaxAge_;
    }

private:
    std::filesystem::path templateRoot_;
    bool templateCacheEnabled_ = true;
    std::vector<StaticFileMapping> staticFiles_;
    bool staticFileCacheEnabled_ = true;
    std::chrono::seconds staticFileCacheMaxAge_ = std::chrono::hours(24);
};

} // namespace drogular
