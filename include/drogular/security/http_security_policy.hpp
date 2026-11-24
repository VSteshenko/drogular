#pragma once

#include <drogular/security/authentication_adapter.hpp>
#include <drogular/security/authorization.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace drogular::security {

// Route policies are opt-in. Public routes still receive an authenticated
// identity when an authentication pipeline has been configured.
enum class HttpAccessMode {
    Public,
    Authenticated,
    Role,
    Permission
};

struct HttpSecurityPolicy final {
    HttpAccessMode mode = HttpAccessMode::Public;
    std::string value;

    static HttpSecurityPolicy authenticated() {
        return {HttpAccessMode::Authenticated, {}};
    }
    static HttpSecurityPolicy role(std::string name) {
        return {HttpAccessMode::Role, std::move(name)};
    }
    static HttpSecurityPolicy permission(std::string name) {
        return {HttpAccessMode::Permission, std::move(name)};
    }
};

[[nodiscard]] inline AuthorizationDecision evaluateHttpPolicy(
    const SecurityContext& context, const HttpSecurityPolicy& policy) noexcept
{
    switch (policy.mode) {
    case HttpAccessMode::Public:
        return AuthorizationDecision::Allowed;
    case HttpAccessMode::Authenticated:
        return Authorization::requireAuthenticated(context);
    case HttpAccessMode::Role:
        return Authorization::requireRole(context, policy.value);
    case HttpAccessMode::Permission:
        return Authorization::requirePermission(context, policy.value);
    }
    return AuthorizationDecision::Forbidden;
}

[[nodiscard]] inline int authorizationHttpStatus(AuthorizationDecision decision) noexcept {
    switch (decision) {
    case AuthorizationDecision::Allowed:
        return 200;
    case AuthorizationDecision::AuthenticationRequired:
        return 401;
    case AuthorizationDecision::Forbidden:
        return 403;
    }
    return 403;
}

[[nodiscard]] inline int authenticationErrorHttpStatus(AuthenticationErrorCode code) noexcept {
    switch (code) {
    case AuthenticationErrorCode::InvalidCredentials:
    case AuthenticationErrorCode::ExpiredCredentials:
        return 401;
    case AuthenticationErrorCode::ProviderUnavailable:
        return 503;
    case AuthenticationErrorCode::InternalError:
    case AuthenticationErrorCode::InvalidAdapterResult:
        return 500;
    }
    return 500;
}

// Shared configuration object: configure it before starting the HTTP server.
struct HttpSecurityConfiguration final {
    std::shared_ptr<const AuthenticationPipeline> pipeline;
};

} // namespace drogular::security