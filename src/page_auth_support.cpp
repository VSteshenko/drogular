#include <drogular/page_auth_support.hpp>
#include <drogular/render_context.hpp>
#include <drogular/auth_support.hpp>

namespace drogular {

bool PageAuthSupport::requireAuthentication(
    RenderContext& context
) {
    const auto authenticated =
        AuthSupport::isAuthenticated(context);

    context.set(
        "loginRequired",
        !authenticated
    );

    return authenticated;
}

bool PageAuthSupport::requireSessionValue(
    RenderContext& context,
    const std::string& key,
    const std::string& expectedValue
) {
    const auto allowed =
        AuthSupport::hasSessionValue(
            context,
            key,
            expectedValue
        );

    context.set(
        "accessDenied",
        !allowed
    );

    return allowed;
}

} // namespace drogular