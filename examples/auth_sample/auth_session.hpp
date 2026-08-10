#pragma once

#include "auth_user.hpp"

#include <drogular/auth_support.hpp>
#include <drogular/page_auth_support.hpp>

#include <optional>
#include <string>

class AuthSession {
public:
    static std::optional<AuthUser> currentUser(
        drogular::RenderContext& context
    ) {
        const auto username =
            drogular::AuthSupport::sessionValue(
                context,
                "username"
            );

        const auto role =
            drogular::AuthSupport::sessionValue(
                context,
                "role"
            );

        if (!username.has_value() ||
            !role.has_value()) {
            return std::nullopt;
        }

        return AuthUser{
            .username = *username,
            .role = *role
        };
    }

    static std::optional<AuthUser> requireCurrentUser(
        drogular::RenderContext& context
    ) {
        if (!drogular::PageAuthSupport::requireAuthentication(
                context
            )) {
            return std::nullopt;
        }

        const auto user = currentUser(context);

        if (!user.has_value()) {
            context.set("loginRequired", true);
        }

        return user;
    }

    static bool isAuthenticated(
        drogular::RenderContext& context
    )
    {
        return currentUser(context).has_value();
    }

    static bool isAdmin(
        drogular::RenderContext& context
    )
    {
        return drogular::AuthSupport::hasSessionValue(
            context,
            "role",
            "admin"
        );
    }
};