#include <drogular/action_context.hpp>
#include <drogular/session_store.hpp>

namespace drogular {

ActionContext::ActionContext(
    drogon::HttpRequestPtr request,
    ApplicationServices* services
)
    : state_(
          std::make_shared<detail::RequestContextState>(
              std::move(request),
              services
          )
      ) {
}

const drogon::HttpRequestPtr& ActionContext::request() const {
    return state_->request();
}

ApplicationServices* ActionContext::services() {
    return state_->services();
}

const ApplicationServices* ActionContext::services() const {
    return state_->services();
}

std::optional<std::string> ActionContext::formValue(
    const std::string& name
) const {
    const auto value = request()->getParameter(name);

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
        request()->getCookie(name);

    if (value.empty()) {
        return std::nullopt;
    }

    return value;
}

std::shared_ptr<Session> ActionContext::existingSession() const {
    const auto* applicationServices = services();

    if (applicationServices == nullptr) {
        return nullptr;
    }

    auto store =
        applicationServices->service<SessionStore>();

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

void ActionContext::setRouteParam(
    const std::string& name,
    const std::string& value
) {
    state_->setRouteParam(name, value);
}

std::optional<std::string> ActionContext::routeParam(
    const std::string& name
) const {
    return state_->routeParam(name);
}

std::string ActionContext::requireRouteParam(
    const std::string& name
) const {
    const auto value =
        routeParam(name);

    if (!value.has_value()) {
        throw ActionContextError(
            "Missing route parameter: " + name
        );
    }

    return *value;
}

std::shared_ptr<Session> ActionContext::session() {
    auto* applicationServices = services();

    if (applicationServices == nullptr) {
        return nullptr;
    }

    auto store =
        applicationServices->requireService<SessionStore>();

    if (const auto existing = existingSession()) {
        return existing;
    }

    return store->create();
}

} // namespace drogular