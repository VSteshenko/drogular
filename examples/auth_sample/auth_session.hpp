#pragma once

#include "auth_user.hpp"

#include <drogular/auth_support.hpp>
#include <drogular/page.hpp>

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

    static bool isAuthenticated(
        drogular::RenderContext& context
    )
    {
        return drogular::AuthSupport::isAuthenticated(
            context
        );
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