#include <drogular/static_file_response.hpp>
#include <drogular/static_file_content_type.hpp>

namespace drogular {

drogon::HttpResponsePtr StaticFileResponse::create(
    const std::filesystem::path& path
) {
    auto response =
        drogon::HttpResponse::newFileResponse(
            path.string()
        );

    response->setContentTypeString(
        StaticFileContentType::fromPath(path)
    );

    return response;
}

} // namespace drogular