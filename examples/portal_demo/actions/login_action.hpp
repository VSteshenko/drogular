#pragma once

#include "../portal_user_repository.hpp"

#include <drogular/action_handler.hpp>

class PortalLoginAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto username =
            context.requireForm<std::string>(
                "username"
            );

        const auto password =
            context.requireForm<std::string>(
                "password"
            );

        auto repository =
            context.requireService<PortalUserRepository>();

        const auto user =
            repository->findByCredentials(
                username,
                password
            );

        if (!user.has_value()) {
            return drogular::ActionResult::redirect(
                "/login"
            );
        }

        auto session =
            context.session();

        session->set(
            "username",
            user->username
        );

        session->set(
            "role",
            user->role
        );

        const auto sessionId =
            session->get("_id").value();

        return drogular::ActionResult::redirect(
            "/dashboard"
        ).cookie(
            "session_id",
            sessionId
        );
    }
};