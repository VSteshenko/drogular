#pragma once

#include "auth_service.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/session_store.hpp>

class LoginAction final : public drogular::ActionHandler {
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
            return drogular::ActionResult::redirect("/login");
        }

        auto authService =
            context.requireService<AuthService>();

        const auto user =
            authService->authenticate(
                context.requireForm<std::string>("username"),
                context.requireForm<std::string>("password")
            );

        if (!user.has_value()) {
            return drogular::ActionResult::redirect("/login");
        }

        auto session =
            context.session();

        session->set(
            "user_id",
            std::to_string(user->id)
        );

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

        return drogular::ActionResult::redirect("/dashboard")
            .cookie("session_id", sessionId);
    }
};