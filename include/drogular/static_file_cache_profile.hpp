#pragma once

namespace drogular {

/**
 * Defines predefined static file caching strategies.
 */
enum class StaticFileCacheProfile {
    /**
     * Disables browser caching and conditional cache headers.
     */
    Disabled,

    /**
     * Disables browser cache storage but keeps validation headers.
     *
     * Useful during development.
     */
    Development,

    /**
     * Enables browser caching and validation headers.
     *
     * Suitable for production static assets.
     */
    Production
};

} // namespace drogular