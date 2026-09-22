#pragma once

#include "features/department_members/providers/department_member_provider.hpp"
#include "features/department_members/ui/portal_department_members_interaction_support.hpp"
#include "features/departments/providers/department_provider.hpp"
#include "features/departments/ui/portal_department_navigation_support.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>

#include <cstdlib>
#include <string>

class PortalAddDepartmentMemberAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto departmentId =
            std::atoi(context.requireRouteParam("id").c_str());
        const auto returnUrl =
            PortalDepartmentNavigationSupport::departmentsReturnUrl(
                context.form<std::string>("returnUrl")
                    .value_or("/departments")
            );
        const auto back =
            PortalDepartmentNavigationSupport::detailsUrl(
                departmentId,
                returnUrl
            );

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
                back + "&error=access_denied")
            ) {
            return *result;
        }

        const auto userId = context.form<int>("userId");
        if (!userId ||
            !context.requireService<PortalDepartmentProvider>()
                ->findById(departmentId) ||
            !context.requireService<PortalUserProvider>()
                ->findById(*userId)
        ) {
            if (context.isInteraction()) {
                return PortalDepartmentMembersInteractionSupport::render(
                    context,
                    departmentId,
                    returnUrl,
                    "invalid_member",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                back + "&error=invalid_member"
            );
        }

        auto memberships =
            context.requireService<PortalDepartmentMemberProvider>();
        if (memberships->find(departmentId, *userId)) {
            if (context.isInteraction()) {
                return PortalDepartmentMembersInteractionSupport::render(
                    context,
                    departmentId,
                    returnUrl,
                    "duplicate_member",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                back + "&error=duplicate_member"
            );
        }

        memberships->addMember(departmentId, *userId);

        if (context.isInteraction()) {
            return PortalDepartmentMembersInteractionSupport::render(
                context,
                departmentId,
                returnUrl,
                "",
                "member_added"
            );
        }

        return drogular::ActionResult::redirect(
            back + "&success=member_added"
        );
    }
};