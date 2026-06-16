#pragma once

#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>

#include <memory>
#include <optional>
#include <string>

namespace drogular {

/**
 * Provides access to request data and application services
 * during action execution.
 */
class ActionContext {
public:
    ActionContext(
        drogon::HttpRequestPtr request,
        ApplicationServices* services
    );

    const drogon::HttpRequestPtr& request() const;

    ApplicationServices* services();

    const ApplicationServices* services() const;

    template <typename T>
    std::shared_ptr<T> service() {
        if (services_ == nullptr) {
            return nullptr;
        }

        return services_->service<T>();
    }

    template <typename T>
    std::shared_ptr<T> requireService() {
        if (services_ == nullptr) {
            throw std::runtime_error("ApplicationServices not set");
        }

        return services_->requireService<T>();
    }

    /**
     * Returns a form parameter value.
     */
    std::optional<std::string> formValue(
        const std::string& name
    ) const;

private:
    drogon::HttpRequestPtr request_;
    ApplicationServices* services_ = nullptr;
};

} // namespace drogular
