#pragma once

#include "features/users/data/portal_user_update.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/url.hpp>

class PortalUpdateUserAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
                drogular::ActionAuthSupport::requireAuthentication(context)) {
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

        const auto id =
            std::atoi(context.requireRouteParam("id").c_str());

        const auto username =
            context.form<std::string>("username").value_or("");

        if (!username.empty()) {
            const auto validation =
                drogular::FormValidator(context)
                    .minLength("username", 2)
                    .validate();

            if (!validation.valid()) {
                // validation failed
                return drogular::ActionResult::redirect(
                    "/users?error=validation&username=" +
                    drogular::Url::encode(username)
                );
            }
        }

        auto repository =
            context.requireService<PortalUserProvider>();

        PortalUserUpdate input;
        input.id = id;

        if (!username.empty()) {
            input.username =
                username;
        }

        const auto role =
            context.form<std::string>("role").value_or("");

        if (!role.empty()) {
            input.role =
                role;
        }

        const auto updated =
            repository->update(
                input
            );

        if (updated.id != id) {
            return drogular::ActionResult::redirect(
                "/users?error=user_not_found"
            );
        }

        return drogular::ActionResult::redirect(
            "/users?success=user_updated"
        );
    }
};