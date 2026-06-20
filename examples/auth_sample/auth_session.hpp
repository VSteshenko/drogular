#pragma once

#include "auth_user.hpp"

#include <drogular/render_context.hpp>
#include <drogular/session_store.hpp>

#include <optional>
#include <string>

class AuthSession {
public:
    static std::optional<AuthUser> currentUser(
        drogular::RenderContext& context
    ) {
        auto sessionStore =
            context.requireService<drogular::SessionStore>();

        // RenderContext has no request/cookie access,
        // so for pages we still need a way to resolve session_id.
        // For now this helper is useful once session_id is passed
        // into context or page request context is available.
        return std::nullopt;
    }

    static std::optional<AuthUser> fromSession(
        const drogular::Session& session
    ) {
        const auto id =
            session.get("user_id");

        const auto username =
            session.get("username");

        const auto role =
            session.get("role");

        if (!id.has_value() ||
            !username.has_value() ||
            !role.has_value()) {
            return std::nullopt;
        }

        return AuthUser{
            .id = std::stoi(*id),
            .username = *username,
            .role = *role
        };
    }
};