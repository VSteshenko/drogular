#pragma once

#include <drogular/static_file_mapping.hpp>
#include <drogular/static_file_cache_profile.hpp>

#include <string>
#include <vector>
#include <utility>
#include <filesystem>
#include <chrono>

namespace drogular {

/**
 * Stores application-wide configuration.
 *
 * ApplicationOptions controls template loading,
 * static file handling, caching and other framework
 * level behavior.
 */
class ApplicationOptions {
public:
    /**
     * Sets the root directory used to resolve template files.
     *
     * Template paths returned from TemplatePage::templatePath()
     * and TemplatePage::layoutPath() are resolved relative to
     * this directory.
     *
     * Example:
     *
     * options.setTemplateRoot(
     *     "templates"
     * );
     */
    void setTemplateRoot(
        std::filesystem::path root
    ) {
        templateRoot_ = std::move(root);
    }

    /**
     * Returns the configured template root directory.
     */
    const std::filesystem::path& templateRoot() const {
        return templateRoot_;
    }

    /**
     * Enables or disables template source caching.
     *
     * When disabled, templates are reloaded from disk on every
     * request. This is useful during development.
     *
     * Enabled by default.
     */
    void setTemplateCacheEnabled(bool enabled) {
        templateCacheEnabled_ = enabled;
    }

    /**
     * Returns whether template source caching is enabled.
     */
    bool templateCacheEnabled() const {
        return templateCacheEnabled_;
    }

    /**
     * Registers a static file mapping.
     *
     * Requests matching the specified route prefix are served
     * from the provided directory.
     *
     * Example:
     *
     * options.addStaticFiles(
     *     "/assets",
     *     "public"
     * );
     *
     * Request:
     *
     * /assets/logo.svg
     *
     * File:
     *
     * public/logo.svg
     */
    void addStaticFiles(
        std::string routePrefix,
        std::filesystem::path directory
    ) {
        staticFiles_.push_back({
            .routePrefix = std::move(routePrefix),
            .directory = std::move(directory)
        });
    }

    /**
     * Returns all registered static file mappings.
     */
    const std::vector<StaticFileMapping>& staticFiles() const {
        return staticFiles_;
    }

    /**
     * Enables or disables Cache-Control headers for static files.
     *
     * Enabled by default.
     */
    void setStaticFileCacheEnabled(bool enabled) {
        staticFileCacheEnabled_ = enabled;
    }

    /**
     * Returns whether Cache-Control headers are enabled
     * for static file responses.
     */
    bool staticFileCacheEnabled() const {
        return staticFileCacheEnabled_;
    }

    /**
     * Sets Cache-Control max-age for static file responses.
     *
     * Example:
     *
     * options.setStaticFileCacheMaxAge(
     *     std::chrono::hours(24)
     * );
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

    /**
     * Enables or disables ETag headers for static file responses.
     *
     * Enabled by default.
     */
    void setStaticFileEtagEnabled(bool enabled) {
        staticFileEtagEnabled_ = enabled;
    }

    /**
     * Returns whether ETag headers are enabled
     * for static file responses.
     */
    bool staticFileEtagEnabled() const {
        return staticFileEtagEnabled_;
    }

    /**
     * Enables or disables Last-Modified headers for static files.
     *
     * Enabled by default.
     */
    void setStaticFileLastModifiedEnabled(bool enabled) {
        staticFileLastModifiedEnabled_ = enabled;
    }

    /**
     * Returns whether Last-Modified headers are enabled
     * for static file responses.
     */
    bool staticFileLastModifiedEnabled() const {
        return staticFileLastModifiedEnabled_;
    }

    /**
     * Applies a predefined static file cache profile.
     */
    void setStaticFileCacheProfile(
        StaticFileCacheProfile profile
    ) {
        switch (profile) {
        case StaticFileCacheProfile::Disabled:
            setStaticFileCacheEnabled(false);
            setStaticFileEtagEnabled(false);
            setStaticFileLastModifiedEnabled(false);
            break;

        case StaticFileCacheProfile::Development:
            setStaticFileCacheEnabled(false);
            setStaticFileEtagEnabled(true);
            setStaticFileLastModifiedEnabled(true);
            break;

        case StaticFileCacheProfile::Production:
            setStaticFileCacheEnabled(true);
            setStaticFileCacheMaxAge(
                std::chrono::hours(24)
            );
            setStaticFileEtagEnabled(true);
            setStaticFileLastModifiedEnabled(true);
            break;
        }
    }

private:
    std::filesystem::path templateRoot_;
    bool templateCacheEnabled_ = true;
    std::vector<StaticFileMapping> staticFiles_;
    bool staticFileCacheEnabled_ = true;
    std::chrono::seconds staticFileCacheMaxAge_ = std::chrono::hours(24);
    bool staticFileEtagEnabled_ = true;
    bool staticFileLastModifiedEnabled_ = true;
};

} // namespace drogular
