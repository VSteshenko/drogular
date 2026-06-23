#include <drogular/static_file_response.hpp>
#include <drogular/static_file_content_type.hpp>
#include <drogular/static_file_etag.hpp>
#include <drogular/static_file_last_modified.hpp>

#include <string>
#include <filesystem>

namespace drogular {

drogon::HttpResponsePtr StaticFileResponse::create(
    const std::filesystem::path& path,
    StaticFileResponseOptions options
) {
    auto response =
        drogon::HttpResponse::newFileResponse(
            path.string()
        );

    response->setContentTypeString(
        StaticFileContentType::fromPath(path)
    );

    if (options.cacheEnabled) {
        response->addHeader(
            "Cache-Control",
            "public, max-age=" +
                std::to_string(
                    options.maxAge.count()
                )
        );
    } else {
        response->addHeader(
            "Cache-Control",
            "no-store"
        );
    }

    if (options.etagEnabled) {
        response->addHeader(
            "ETag",
            StaticFileEtag::create(path)
        );
    }

    if (options.lastModifiedEnabled) {
        response->addHeader(
            "Last-Modified",
            StaticFileLastModified::create(path)
        );
    }

    return response;
}

drogon::HttpResponsePtr StaticFileResponse::notModified(
    std::string etag,
    std::string lastModified
) {
    auto response =
        drogon::HttpResponse::newHttpResponse();

    response->setStatusCode(
        drogon::k304NotModified
    );

    if (!etag.empty()) {
        response->addHeader("ETag", etag);
    }

    if (!lastModified.empty()) {
        response->addHeader("Last-Modified", lastModified);
    }

    return response;
}

} // namespace drogular