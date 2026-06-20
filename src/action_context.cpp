#include <drogular/action_context.hpp>
#include <drogular/session_store.hpp>

namespace drogular {

ActionContext::ActionContext(
    drogon::HttpRequestPtr request,
    ApplicationServices* services
)
    : request_(std::move(request)),
      services_(services) {
}

const drogon::HttpRequestPtr& ActionContext::request() const {
    return request_;
}

ApplicationServices* ActionContext::services() {
    return services_;
}

const ApplicationServices* ActionContext::services() const {
    return services_;
}

std::optional<std::string> ActionContext::formValue(
    const std::string& name
) const {
    const auto value = request_->getParameter(name);

    if (value.empty()) {
        return std::nullopt;
    }

    return value;
}

std::string ActionContext::requireFormValue(
    const std::string& name
) const {
    const auto value = formValue(name);

    if (!value.has_value()) {
        throw ActionValidationError(
            "Missing required form value: " + name
        );
    }

    return *value;
}

std::optional<std::string> ActionContext::cookie(
    const std::string& name
) const {
    const auto value =
        request_->getCookie(name);

    if (value.empty()) {
        return std::nullopt;
    }

    return value;
}

std::shared_ptr<Session> ActionContext::existingSession() const {
    if (services_ == nullptr) {
        return nullptr;
    }

    auto store =
        services_->service<SessionStore>();

    if (store == nullptr) {
        return nullptr;
    }

    const auto sessionId =
        cookie("session_id");

    if (!sessionId.has_value()) {
        return nullptr;
    }

    return store->get(*sessionId);
}

std::shared_ptr<Session> ActionContext::session() {
    if (services_ == nullptr) {
        return nullptr;
    }

    auto store =
        services_->requireService<SessionStore>();

    if (const auto existing = existingSession()) {
        return existing;
    }

    return store->create();
}

} // namespace drogular