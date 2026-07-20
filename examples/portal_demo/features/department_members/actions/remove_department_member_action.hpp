#pragma once

#include "features/department_members/providers/department_member_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>

#include <cstdlib>

class PortalRemoveDepartmentMemberAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        const auto departmentId =
            std::atoi(context.requireRouteParam("id").c_str());

        const auto userId =
            std::atoi(context.requireRouteParam("userId").c_str());

        const auto back =
            "/departments/" + std::to_string(departmentId);

        if (const auto result =
            drogular::ActionAuthSupport::requireAuthentication(context)
        ) {
            return *result;
        }
        if (const auto result =
            drogular::ActionAuthSupport::requireSessionValue(
                context,
                "role",
                "admin",
                back + "?error=access_denied")
        ) {
            return *result;
        }

        if (!context.requireService<PortalDepartmentMemberProvider>()->removeMember(
                departmentId,
                userId)
            ) {
            return drogular::ActionResult::redirect(back + "?error=member_not_found");
        }

        return drogular::ActionResult::redirect(back + "?success=member_removed");
    }
};