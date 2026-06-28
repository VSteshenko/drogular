#include <drogular/router.hpp>
#include <drogular/services.hpp>
#include <drogular/page.hpp>
#include <drogular/action_response.hpp>
#include <drogular/static_file_resolver.hpp>
#include <drogular/static_file_response.hpp>
#include <drogular/static_file_etag.hpp>
#include <drogular/static_file_last_modified.hpp>
#include <drogular/render_context.hpp>

#include <drogon/drogon.h>

#include <filesystem>

namespace drogular {

namespace {

bool isInsideDirectory(
    const std::filesystem::path& root,
    const std::filesystem::path& requested
) {
    const auto canonicalRoot =
        std::filesystem::weakly_canonical(root);

    const auto canonicalRequested =
        std::filesystem::weakly_canonical(requested);

    const auto rootString =
        canonicalRoot.string();

    const auto requestedString =
        canonicalRequested.string();

    return requestedString == rootString ||
           requestedString.starts_with(
               rootString + std::string(1, std::filesystem::path::preferred_separator)
           );
}

} // namespace

Router::Router(ApplicationServices* services)
    : services_(services) {
}

void Router::page(const std::string& path, std::shared_ptr<Page> page) {
    auto* services = services_;

    drogon::app().registerHandler(
        path,
        [page, services](const drogon::HttpRequestPtr& request,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            RenderContext context;
            context.setServices(services);
            context.setRequest(request);

            page->onInit(context);

            auto response = drogon::HttpResponse::newHttpResponse();
            response->setContentTypeCode(drogon::CT_TEXT_HTML);
            response->setBody(page->render(context));

            callback(response);
        }
    );
}

void Router::action(const std::string& path, std::shared_ptr<ActionHandler> action) {
    auto* services = services_;

    drogon::app().registerHandler(
        path,
        [action, services](
            const drogon::HttpRequestPtr& request,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) {
            ActionContext context(
                request,
                services
            );

            const auto result =
                action->handle(context);

            callback(
                toHttpResponse(result)
            );
        },
        {drogon::Post}
    );
}

void Router::staticFiles(
    const std::string& routePrefix,
    const std::filesystem::path& directory
) {
    auto normalizedPrefix = routePrefix;

    if (!normalizedPrefix.empty() &&
        normalizedPrefix.back() == '/') {
        normalizedPrefix.pop_back();
        }

    const auto rootDirectory =
        std::filesystem::weakly_canonical(directory);

    auto* options =
        services_ != nullptr
            ? services_->options()
            : nullptr;

    drogon::app().registerHandler(
        normalizedPrefix + "/{filePath}",
        [rootDirectory, options](
            const drogon::HttpRequestPtr& request,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            const std::string& filePath
        ) {
            const auto requestedPath =
                rootDirectory / filePath;

            if (!isInsideDirectory(
                    rootDirectory,
                    requestedPath
                )) {
                auto response =
                    drogon::HttpResponse::newHttpResponse();

                response->setStatusCode(
                    drogon::k403Forbidden
                );

                callback(response);
                return;
            }

            if (!std::filesystem::exists(requestedPath) ||
                !std::filesystem::is_regular_file(requestedPath)) {
                auto response =
                    drogon::HttpResponse::newHttpResponse();

                response->setStatusCode(
                    drogon::k404NotFound
                );

                callback(response);
                return;
            }

            StaticFileResolver resolver(rootDirectory);

            const auto resolved =
                resolver.resolve(filePath);

            if (!resolved.has_value()) {
                auto response =
                    drogon::HttpResponse::newHttpResponse();

                response->setStatusCode(drogon::k404NotFound);

                callback(response);
                return;
            }

            StaticFileResponseOptions responseOptions;

            if (options != nullptr) {
                responseOptions.cacheEnabled =
                    options->staticFileCacheEnabled();

                responseOptions.maxAge =
                    options->staticFileCacheMaxAge();

                responseOptions.etagEnabled =
                    options->staticFileEtagEnabled();

                responseOptions.lastModifiedEnabled =
                    options->staticFileLastModifiedEnabled();
            }

            if (responseOptions.etagEnabled) {
                const auto etag =
                    StaticFileEtag::create(
                        *resolved
                    );

                const auto requestEtag =
                    request->getHeader("If-None-Match");

                if (requestEtag == etag) {
                    callback(
                        StaticFileResponse::notModified(etag)
                    );

                    return;
                }
            }

            if (responseOptions.lastModifiedEnabled) {
                const auto lastModified =
                    drogular::StaticFileLastModified::create(
                        *resolved
                    );

                const auto requestLastModified =
                    request->getHeader("If-Modified-Since");

                if (requestLastModified == lastModified) {
                    callback(
                        drogular::StaticFileResponse::notModified(
                            responseOptions.etagEnabled
                                ? drogular::StaticFileEtag::create(*resolved)
                                : ""
                        )
                    );

                    return;
                }
            }

            auto response =
                StaticFileResponse::create(
                    *resolved,
                    responseOptions
                );

            callback(response);
        },
        {drogon::Get}
    );
}

void Router::serviceWorker(
    const std::filesystem::path& path
) {
    drogon::app().registerHandler(
        "/service-worker.js",
        [path](
            const drogon::HttpRequestPtr&,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) {
            auto response =
                drogular::StaticFileResponse::create(path);

            response->setContentTypeString(
                "application/javascript"
            );

            response->addHeader(
                "Service-Worker-Allowed",
                "/"
            );

            callback(response);
        },
        {drogon::Get}
    );
}

} // namespace drogular