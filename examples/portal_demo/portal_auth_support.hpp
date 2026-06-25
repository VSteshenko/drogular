#pragma once

#include "portal_user.hpp"

#include <drogular/component.hpp>
#include <drogular/session_store.hpp>

#include <optional>
#include <string>

class PortalAuthSupport {
public:
    static std::optional<PortalUser> currentUser(
        drogular::RenderContext& context
    ) {
        const auto sessionId =
            context.cookie("session_id");

        if (!sessionId.has_value()) {
            return std::nullopt;
        }

        auto store =
            context.requireService<drogular::SessionStore>();

        const auto session =
            store->get(*sessionId);

        if (session == nullptr) {
            return std::nullopt;
        }

        const auto username =
            session->get("username");

        const auto role =
            session->get("role");

        if (!username.has_value() ||
            !role.has_value()) {
            return std::nullopt;
        }

        return PortalUser{
            .username = *username,
            .password = "",
            .role = *role
        };
    }

    static bool isAuthenticated(
        drogular::RenderContext& context
    ) {
        return currentUser(context).has_value();
    }

    static bool hasRole(
        drogular::RenderContext& context,
        const std::string& role
    ) {
        const auto user =
            currentUser(context);

        return user.has_value() &&
               user->role == role;
    }
};