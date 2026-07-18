#pragma once

#include "data/models/portal_role_create.hpp"
#include "providers/role_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/url.hpp>

#include <algorithm>
#include <string>

class PortalCreateRoleAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
                drogular::ActionAuthSupport::
                    requireAuthentication(context)) {
            return *result;
        }

        if (const auto result =
                drogular::ActionAuthSupport::
                    requireSessionValue(
                        context,
                        "role",
                        "admin",
                        "/roles?error=access_denied"
                    )) {
            return *result;
        }

        const auto validation =
            drogular::FormValidator(context)
                .required("code")
                .minLength("code", 2)
                .required("title")
                .minLength("title", 2)
                .validate();

        const auto code =
            context.form<std::string>("code")
                .value_or("");

        const auto title =
            context.form<std::string>("title")
                .value_or("");

        if (!validation.valid()) {
            return drogular::ActionResult::redirect(
                "/roles?error=validation"
                "&code=" +
                drogular::Url::encode(code) +
                "&title=" +
                drogular::Url::encode(title)
            );
        }

        auto roles =
            context.requireService<PortalRoleProvider>();

        const auto existing = roles->all();

        const auto duplicate =
            std::any_of(
                existing.begin(),
                existing.end(),
                [&code](const PortalRole& role) {
                    return role.code == code;
                }
            );

        if (duplicate) {
            return drogular::ActionResult::redirect(
                "/roles?error=duplicate_code"
                "&code=" +
                drogular::Url::encode(code) +
                "&title=" +
                drogular::Url::encode(title)
            );
        }

        PortalRoleCreate input;

        input.code = context.requireForm<std::string>("code");
        input.title = context.requireForm<std::string>("title");

        roles->create(input);

        return drogular::ActionResult::redirect(
            "/roles?success=role_created"
        );
    }
};