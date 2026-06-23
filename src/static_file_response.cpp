#include <drogular/static_file_response.hpp>
#include <drogular/static_file_content_type.hpp>

#include <string>

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

    return response;
}

} // namespace drogular