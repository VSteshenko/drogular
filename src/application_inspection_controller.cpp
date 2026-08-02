#include <drogular/application_inspection_controller.hpp>
#include <drogular/application_inspection.hpp>

namespace drogular {

ApplicationInspectionController::ApplicationInspectionController(
    ApplicationServices* services
) : services_(services) {
}

void ApplicationInspectionController::handle(
    const drogon::HttpRequestPtr&,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback
) const {
    const auto provider =
        services_ != nullptr
            ? services_->service<ApplicationInspectionProvider>()
            : nullptr;

    if (provider == nullptr) {
        auto response = drogon::HttpResponse::newHttpResponse();
        response->setStatusCode(drogon::k503ServiceUnavailable);
        callback(response);
        return;
    }

    const auto inspection = (*provider)();

    callback(
        drogon::HttpResponse::newHttpJsonResponse(
            toJson(inspection)
        )
    );
}

} // namespace drogular