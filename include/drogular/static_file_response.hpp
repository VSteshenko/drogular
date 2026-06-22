#pragma once

#include <drogon/HttpResponse.h>

#include <filesystem>

namespace drogular {

class StaticFileResponse {
public:
    /**
     * Creates an HTTP file response with detected content type.
     */
    static drogon::HttpResponsePtr create(
        const std::filesystem::path& path
    );
};

} // namespace drogular