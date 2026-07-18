#pragma once

#include "providers/project_type_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>

#include <cstdlib>
#include <string>

class PortalDeleteProjectTypeAction final
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
                    "/project-types?error=access_denied"
                )) {
            return *result;
                }

        const auto id =
            std::atoi(
                context.requireRouteParam("id").c_str()
            );

        auto projectTypes =
            context.requireService<
                PortalProjectTypeProvider
            >();

        if (!projectTypes->remove(id)) {
            return drogular::ActionResult::redirect(
                "/project-types?error=project_type_in_use"
            );
        }

        return drogular::ActionResult::redirect(
            "/project-types?success=project_type_deleted"
        );
    }
};