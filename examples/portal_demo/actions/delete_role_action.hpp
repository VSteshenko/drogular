#pragma once

#include "providers/role_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>

#include <cstdlib>

class PortalDeleteRoleAction final
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

        auto roles =
            context.requireService<PortalRoleProvider>();

        if (!roles->remove(id)) {
            return drogular::ActionResult::redirect(
                "/roles?error=role_in_use"
            );
        }

        return drogular::ActionResult::redirect(
            "/roles?success=role_deleted"
        );
    }
};