#pragma once

#include "features/department_members/providers/department_member_provider.hpp"
#include "features/departments/providers/department_provider.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>

#include <cstdlib>

class PortalAddDepartmentMemberAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto departmentId =
            std::atoi(context.requireRouteParam("id").c_str());

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

        const auto userId =
            context.form<int>("userId");
        if (!userId ||
            !context.requireService<PortalDepartmentProvider>()->findById(departmentId) ||
            !context.requireService<PortalUserProvider>()->findById(*userId)
        ) {
            return drogular::ActionResult::redirect(back + "?error=invalid_member");
        }

        auto memberships =
            context.requireService<PortalDepartmentMemberProvider>();
        if (memberships->find(departmentId, *userId)) {
            return drogular::ActionResult::redirect(back + "?error=duplicate_member");
        }
        memberships->addMember(departmentId, *userId);
        return drogular::ActionResult::redirect(back + "?success=member_added");
    }
};