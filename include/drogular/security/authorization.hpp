#pragma once

#include <drogular/security/security_context.hpp>

#include <algorithm>
#include <string_view>
#include <vector>

namespace drogular::security {

// A framework-independent authorization outcome. HTTP adapters may translate
// AuthenticationRequired to 401 and Forbidden to 403; this module does not
// perform redirects or modify responses.
enum class AuthorizationDecision {
    Allowed,
    AuthenticationRequired,
    Forbidden
};

// Authorization policy v0.27: role names match exactly, and roles are not
// automatically elevated to permissions. An authenticated principal does not
// imply access to any resource.
class Authorization final {
public:
    [[nodiscard]] static AuthorizationDecision requireAuthenticated(
        const SecurityContext& context) noexcept
    {
        return context.authenticated()
            ? AuthorizationDecision::Allowed
            : AuthorizationDecision::AuthenticationRequired;
    }

    [[nodiscard]] static AuthorizationDecision requireRole(
        const SecurityContext& context, std::string_view role) noexcept
    {
        if (!context.authenticated()) {
            return AuthorizationDecision::AuthenticationRequired;
        }
        const auto principal = context.principal();
        return !role.empty() && principal && principal->hasRole(role)
            ? AuthorizationDecision::Allowed
            : AuthorizationDecision::Forbidden;
    }

    // Permission claims are a convention for verified authentication adapters:
    // Claim "permissions" must be a vector<string>. No coercion or wildcard
    // matching is performed. Untrusted self-declared claims MUST NOT be used.
    [[nodiscard]] static AuthorizationDecision requirePermission(
        const SecurityContext& context, std::string_view permission) noexcept {
        if (!context.authenticated()) {
            return AuthorizationDecision::AuthenticationRequired;
        }
        const auto principal = context.principal();
        if (!principal || permission.empty()) {
            return AuthorizationDecision::Forbidden;
        }
        const auto* claim = principal->claims().find("permissions");
        const auto* permissions = claim
            ? std::get_if<std::vector<std::string>>(claim)
            : nullptr;
        if (!permissions) {
            return AuthorizationDecision::Forbidden;
        }
        return std::any_of(permissions->begin(), permissions->end(),
                           [permission](const std::string& candidate) {
                               return candidate == permission;
                           })
            ? AuthorizationDecision::Allowed
            : AuthorizationDecision::Forbidden;
    }
};

} // namespace drogular::security