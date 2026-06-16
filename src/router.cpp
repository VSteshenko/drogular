#include <drogular/router.hpp>
#include <drogular/services.hpp>
#include <drogular/page.hpp>
#include <drogular/action_response.hpp>

#include <drogon/drogon.h>

namespace drogular {

Router::Router(ApplicationServices* services)
    : services_(services) {
}

void Router::page(const std::string& path, std::shared_ptr<Page> page) {
    auto* services = services_;

    drogon::app().registerHandler(
        path,
        [page, services](const drogon::HttpRequestPtr&,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            RenderContext context;
            context.setServices(services);

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

} // namespace drogular
