#pragma once

#include <drogular/services.hpp>
#include <drogular/session.hpp>
#include <drogular/action_validation_error.hpp>

#include <drogon/HttpRequest.h>

#include <memory>
#include <optional>
#include <string>
#include <stdexcept>
#include <type_traits>

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

    /**
     * Returns a required form parameter value.
     *
     * Throws when the parameter is missing or empty.
     */
    std::string requireFormValue(
        const std::string& name
    ) const;

    /**
     * Returns a typed form parameter value.
     */
    template <typename T>
    std::optional<T> form(const std::string& name) const {
        const auto value = formValue(name);

        if (!value.has_value()) {
            return std::nullopt;
        }

        if constexpr (std::is_same_v<T, std::string>) {
            return *value;
        } else if constexpr (std::is_same_v<T, int>) {
            try {
                return std::stoi(*value);
            } catch (...) {
                return std::nullopt;
            }
        } else if constexpr (std::is_same_v<T, double>) {
            try {
                return std::stod(*value);
            } catch (...) {
                return std::nullopt;
            }
        } else if constexpr (std::is_same_v<T, bool>) {
            return *value == "true" ||
                   *value == "1" ||
                   *value == "on";
        } else {
            static_assert(
                std::is_same_v<T, std::string> ||
                std::is_same_v<T, int> ||
                std::is_same_v<T, double> ||
                std::is_same_v<T, bool>,
                "Unsupported form value type"
            );
        }
    }

    /**
     * Returns a required typed form parameter value.
     *
     * Throws ActionValidationError when the value is missing
     * or cannot be converted to the requested type.
     */
    template <typename T>
    T requireForm(const std::string& name) const {
        const auto value = form<T>(name);

        if (!value.has_value()) {
            throw ActionValidationError(
                "Invalid or missing form value: " + name
            );
        }

        return *value;
    }

    std::optional<std::string> cookie(
        const std::string& name
    ) const;

    std::shared_ptr<Session> session();

    std::shared_ptr<Session> existingSession() const;

private:
    drogon::HttpRequestPtr request_;
    ApplicationServices* services_ = nullptr;
};

} // namespace drogular