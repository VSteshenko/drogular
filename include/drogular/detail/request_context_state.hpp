#pragma once

#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace drogular::detail {

/**
 * Shared request-scoped state used by execution contexts.
 *
 * ActionContext and RenderContext remain separate public abstractions.
 * This object only owns the state whose lifetime is the HTTP request:
 * request, application services, scoped service cache, and route parameters.
 */
class RequestContextState final {
public:
    RequestContextState(
        drogon::HttpRequestPtr request = nullptr,
        ApplicationServices* services = nullptr
    )
        : request_(std::move(request)),
          services_(services),
          serviceScope_(std::make_shared<ServiceScope>()) {
    }

    const drogon::HttpRequestPtr& request() const {
        return request_;
    }

    void setRequest(
        drogon::HttpRequestPtr request
    ) {
        request_ = std::move(request);
    }

    ApplicationServices* services() {
        return services_;
    }

    const ApplicationServices* services() const {
        return services_;
    }

    void setServices(
        ApplicationServices* services
    ) {
        services_ = services;
    }

    ServiceScope& serviceScope() {
        return *serviceScope_;
    }

    const ServiceScope& serviceScope() const {
        return *serviceScope_;
    }

    void setRouteParam(
        const std::string& name,
        const std::string& value
    ) {
        routeParams_[name] = value;
    }

    std::optional<std::string> routeParam(
        const std::string& name
    ) const {
        const auto found = routeParams_.find(name);

        if (found == routeParams_.end()) {
            return std::nullopt;
        }

        return found->second;
    }

private:
    drogon::HttpRequestPtr request_;
    ApplicationServices* services_ = nullptr;
    std::shared_ptr<ServiceScope> serviceScope_;
    std::unordered_map<std::string, std::string> routeParams_;
};

} // namespace drogular::detail