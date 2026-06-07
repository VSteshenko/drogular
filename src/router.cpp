#include <drogular/router.hpp>
#include <drogular/page.hpp>

#include <drogon/drogon.h>

namespace drogular {

void Router::page(const std::string& path, std::shared_ptr<Page> page) {
    drogon::app().registerHandler(
        path,
        [page](const drogon::HttpRequestPtr&,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            auto response = drogon::HttpResponse::newHttpResponse();
            response->setContentTypeCode(drogon::CT_TEXT_HTML);
            response->setBody(page->render());
            callback(response);
        }
    );
}

} // namespace drogular
