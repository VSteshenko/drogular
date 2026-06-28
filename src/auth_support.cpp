#include <drogular/auth_support.hpp>
#include <drogular/render_context.hpp>
#include <drogular/session_store.hpp>

namespace drogular {

std::optional<std::string> AuthSupport::sessionValue(
    RenderContext& context,
    const std::string& key
) {
    const auto sessionId =
        context.cookie("session_id");

    if (!sessionId.has_value()) {
        return std::nullopt;
    }

    auto store =
        context.requireService<SessionStore>();

    const auto session =
        store->get(*sessionId);

    if (session == nullptr) {
        return std::nullopt;
    }

    return session->get(key);
}

bool AuthSupport::isAuthenticated(
    RenderContext& context
) {
    return sessionValue(
        context,
        "username"
    ).has_value();
}

bool AuthSupport::hasSessionValue(
    RenderContext& context,
    const std::string& key,
    const std::string& expectedValue
) {
    const auto value =
        sessionValue(context, key);

    return value.has_value() &&
           *value == expectedValue;
}

} // namespace drogular