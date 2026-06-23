#include <drogular/static_file_response.hpp>
#include <drogular/static_file_content_type.hpp>
#include <drogular/static_file_etag.hpp>

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

    return response;
}

drogon::HttpResponsePtr StaticFileResponse::notModified(
    const std::string& etag
) {
    auto response =
        drogon::HttpResponse::newHttpResponse();

    response->setStatusCode(
        drogon::k304NotModified
    );

    response->addHeader(
        "ETag",
        etag
    );

    return response;
}

} // namespace drogular