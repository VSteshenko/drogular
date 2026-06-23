#pragma once

#include <drogon/HttpResponse.h>

#include <filesystem>
#include <chrono>

namespace drogular {

/**
 * Configures HTTP headers for static file responses.
 */
struct StaticFileResponseOptions {
    /**
     * Enables Cache-Control headers.
     */
    bool cacheEnabled = true;

    /**
     * Max age used when cache headers are enabled.
     */
    std::chrono::seconds maxAge =
        std::chrono::hours(24);

    /**
     * Enables ETag headers.
     */
    bool etagEnabled = true;

    /**
     * Enables Last-Modified headers.
     */
    bool lastModifiedEnabled = true;
};

class StaticFileResponse {
public:
    /**
     * Creates an HTTP file response with content type,
     * cache headers and optional ETag.
     */
    static drogon::HttpResponsePtr create(
        const std::filesystem::path& path,
        StaticFileResponseOptions options = {}
    );

    /**
     * Creates a 304 Not Modified response.
     *
     * Used when the client already has the current
     * version of a cached resource.
     */
    static drogon::HttpResponsePtr notModified(
        std::string etag = "",
        std::string lastModified = ""
    );
};

} // namespace drogular