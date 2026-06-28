#pragma once

#include "../portal_user_repository.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/url.hpp>
#include <drogular/action_auth_support.hpp>

class PortalCreateUserAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
                drogular::ActionAuthSupport::requireAuthentication(
                    context
                )) {
            return *result;
        }

        if (const auto result =
                drogular::ActionAuthSupport::requireSessionValue(
                    context,
                    "role",
                    "admin",
                    "/users?error=access_denied"
                )) {
            return *result;
        }

        const auto validation =
            drogular::FormValidator(context)
                .required("username")
                .minLength("username", 2)
                .required("password")
                .minLength("password", 3)
                .required("role")
                .validate();

        const auto username =
            context.form<std::string>("username")
                .value_or("");

        if (!validation.valid()) {
            // validation failed
            return drogular::ActionResult::redirect(
                "/users?error=validation&username=" +
                drogular::Url::encode(username)
            );
        }

        const auto password =
            context.requireForm<std::string>("password");

        const auto role =
            context.requireForm<std::string>("role");

        auto repository =
            context.requireService<PortalUserRepository>();

        if (repository->exists(username)) {
            return drogular::ActionResult::redirect(
                "/users?error=duplicate_user&username=" +
                drogular::Url::encode(username)
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