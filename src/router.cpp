#include <drogular/router.hpp>
#include <drogular/services.hpp>
#include <drogular/page.hpp>

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

} // namespace drogular
