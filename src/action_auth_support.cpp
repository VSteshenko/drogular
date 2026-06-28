#include <drogular/action_auth_support.hpp>

namespace drogular {

std::optional<ActionResult> ActionAuthSupport::requireAuthentication(
    ActionContext& context,
    const std::string& redirectPath
) {
    const auto session =
        context.existingSession();

    if (session == nullptr) {
        return ActionResult::redirect(
            redirectPath
        );
    }

    return std::nullopt;
}

std::optional<ActionResult> ActionAuthSupport::requireSessionValue(
    ActionContext& context,
    const std::string& key,
    const std::string& expectedValue,
    const std::string& redirectPath
) {
    const auto session =
        context.existingSession();

    if (session == nullptr) {
        return ActionResult::redirect(
            redirectPath
        );
    }

    const auto value =
        session->get(key);

    if (!value.has_value() ||
        *value != expectedValue) {
        return ActionResult::redirect(
            redirectPath
        );
    }

    return std::nullopt;
}

} // namespace drogular