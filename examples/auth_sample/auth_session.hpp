#pragma once

#include "auth_user.hpp"

#include <drogular/component.hpp>
#include <drogular/session.hpp>
#include <drogular/session_store.hpp>

#include <optional>
#include <string>

class AuthSession {
public:
    static std::optional<AuthUser> currentUser(
        drogular::RenderContext& context
    ) {
        const auto sessionId =
            context.cookie("session_id");

        if (!sessionId.has_value()) {
            return std::nullopt;
        }

        auto sessionStore =
            context.requireService<drogular::SessionStore>();

        const auto session =
            sessionStore->get(*sessionId);

        if (session == nullptr) {
            return std::nullopt;
        }

        return fromSession(*session);
    }

    static std::optional<AuthUser> fromSession(
        const drogular::Session& session
    ) {
        const auto id = session.get("user_id");
        const auto username = session.get("username");
        const auto role = session.get("role");

        if (!id || !username || !role) {
            return std::nullopt;
        }

        return AuthUser{
            .id = std::stoi(*id),
            .username = *username,
            .role = *role
        };
    }
};