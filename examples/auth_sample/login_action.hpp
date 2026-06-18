#pragma once

#include "auth_service.hpp"
#include "auth_store.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>

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

        auto authStore =
            context.requireService<AuthStore>();

        const auto user =
            authService->authenticate(
                context.requireForm<std::string>("username"),
                context.requireForm<std::string>("password")
            );

        if (!user.has_value()) {
            return drogular::ActionResult::redirect("/login");
        }

        authStore->currentUser.set(*user);

        return drogular::ActionResult::redirect("/dashboard");
    }
};