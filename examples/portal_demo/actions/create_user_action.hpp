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
            // not logged in
            return drogular::ActionResult::redirect(
                "/login"
            );
        }

        const auto currentRole =
            session->get("role");

        if (!currentRole.has_value() || *currentRole != "admin") {
            // not admin
            return drogular::ActionResult::redirect(
                "/users?error=access_denied"
            );
        }

        const auto validation =
            drogular::FormValidator(context)
                .required("username")
                .minLength("username", 2)
                .required("password")
                .minLength("password", 3)
                .required("role")
                .validate();

        if (!validation.valid()) {
            // validation failed
            return drogular::ActionResult::redirect(
                "/users?error=validation"
            );
        }

        const auto username =
            context.requireForm<std::string>("username");

        const auto password =
            context.requireForm<std::string>("password");

        const auto role =
            context.requireForm<std::string>("role");

        auto repository =
            context.requireService<PortalUserRepository>();

        if (repository->exists(username)) {
            const auto username =
                context.form<std::string>("username")
                    .value_or("");

            return drogular::ActionResult::redirect(
                "/users?error=validation&username=" + username
            );
        }

        repository->create(
            username,
            password,
            role
        );

        // success
        return drogular::ActionResult::redirect(
            "/users?success=user_created"
        );
    }
};