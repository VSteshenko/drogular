#pragma once

#include "../portal_user_repository.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>

class PortalCreateUserAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto session =
            context.existingSession();

        if (session == nullptr) {
            return drogular::ActionResult::redirect(
                "/login"
            );
        }

        const auto role =
            session->get("role");

        if (!role.has_value() || *role != "admin") {
            return drogular::ActionResult::redirect(
                "/users"
            );
        }

        const auto validation =
            drogular::FormValidator(context)
                .required("username")
                .minLength("username", 2)
                .required("role")
                .validate();

        if (!validation.valid()) {
            return drogular::ActionResult::redirect(
                "/users"
            );
        }

        auto repository =
            context.requireService<PortalUserRepository>();

        repository->create(
            context.requireForm<std::string>("username"),
            context.requireForm<std::string>("role")
        );

        return drogular::ActionResult::redirect(
            "/users"
        );
    }
};