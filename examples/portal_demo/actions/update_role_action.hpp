#pragma once

#include "../data/models/portal_role_update.hpp"
#include "../providers/role_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>

#include <algorithm>
#include <cstdlib>
#include <string>

class PortalUpdateRoleAction final
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

        const auto id =
            std::atoi(
                context.requireRouteParam("id").c_str()
            );

        const auto code =
            context.form<std::string>("code");

        const auto title =
            context.form<std::string>("title");

        drogular::FormValidator validator(context);

        if (code.has_value()) {
            validator.required("code");
            validator.minLength("code", 2);
        }

        if (title.has_value()) {
            validator.required("title");
            validator.minLength("title", 2);
        }

        const auto validation =
            validator.validate();

        if (!validation.valid()) {
            return drogular::ActionResult::redirect(
                "/roles/" +
                std::to_string(id) +
                "/edit?error=validation"
            );
        }

        auto roles =
            context.requireService<PortalRoleProvider>();

        const auto all =
            roles->all();

        const auto duplicate =
            std::any_of(
                all.begin(),
                all.end(),
                [id, &code](const PortalRole& role) {
                    return code.has_value() &&
                           role.id != id &&
                           role.code == *code;
                }
            );

        if (duplicate) {
            return drogular::ActionResult::redirect(
                "/roles/" +
                std::to_string(id) +
                "/edit?error=duplicate_code"
            );
        }

        PortalRoleUpdate input;
        input.id = id;

        if (code.has_value()) {
            input.code = *code;
        }

        if (title.has_value()) {
            input.title = *title;
        }

        const auto updated =
            roles->update(input);

        if (updated.id != id) {
            return drogular::ActionResult::redirect(
                "/roles?error=not_found"
            );
        }

        return drogular::ActionResult::redirect(
            "/roles?success=role_updated"
        );
    }
};