#pragma once

#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

#include <functional>

namespace drogular {

class ApplicationInspectionController {
public:
    static constexpr const char* Path = "/__drogular/inspection";

    explicit ApplicationInspectionController(
        ApplicationServices* services
    );

    void handle(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    ) const;

private:
    ApplicationServices* services_ = nullptr;
};

} // namespace drogular