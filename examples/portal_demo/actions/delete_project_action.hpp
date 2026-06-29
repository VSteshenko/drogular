#pragma once

#include "../providers/project_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>

#include <cstdlib>
#include <string>

class PortalDeleteProjectAction final
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

        const auto id =
            std::atoi(
                context.requireRouteParam("id").c_str()
            );

        auto repository =
            context.requireService<PortalProjectProvider>();

        const auto removed =
            repository->remove(id);

        if (!removed) {
            return drogular::ActionResult::redirect(
                "/projects?error=not_found"
            );
        }

        return drogular::ActionResult::redirect(
            "/projects?success=project_deleted"
        );
    }
};