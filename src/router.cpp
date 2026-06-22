#include <drogular/router.hpp>
#include <drogular/services.hpp>
#include <drogular/page.hpp>
#include <drogular/action_response.hpp>

#include <drogon/drogon.h>

#include <filesystem>

namespace drogular {

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

    drogon::app().registerHandler(
        normalizedPrefix + "/{filePath}",
        [normalizedPrefix, directory](
            const drogon::HttpRequestPtr& request,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            const std::string& filePath
        ) {
            const auto fullPath =
                directory / filePath;

            auto response =
                drogon::HttpResponse::newFileResponse(
                    fullPath.string()
                );

            callback(response);
        },
        {drogon::Get}
    );
}

} // namespace drogular
