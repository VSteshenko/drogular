#pragma once

#include "../portal_memory_user_provider.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>

class PortalLoginAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto validation =
            drogular::FormValidator(context)
                .required("username")
                .required("password")
                .validate();

        if (!validation.valid()) {
            return drogular::ActionResult::redirect(
                "/login?error=missing_credentials"
            );
        }

        const auto username =
            context.requireForm<std::string>("username");

        const auto password =
            context.requireForm<std::string>("password");

        auto repository =
            context.requireService<PortalUserProvider>();

        const auto user =
            repository->findByCredentials(
                username,
                password
            );

        if (!user.has_value()) {
            return drogular::ActionResult::redirect(
                "/login?error=invalid_credentials"
            );
        }

        auto session =
            context.session();

        session->set("username", user->username);
        session->set("role", user->role);

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