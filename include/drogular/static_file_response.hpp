#pragma once

#include <drogon/HttpResponse.h>

#include <filesystem>
#include <chrono>

namespace drogular {

struct StaticFileResponseOptions {
    bool cacheEnabled = true;

    std::chrono::seconds maxAge =
        std::chrono::hours(24);
};

class StaticFileResponse {
public:
    /**
     * Creates an HTTP file response with detected content type.
     */
    static drogon::HttpResponsePtr create(
      const std::filesystem::path& path,
        StaticFileResponseOptions options = {}
    );
};

} // namespace drogular