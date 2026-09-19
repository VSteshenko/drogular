#pragma once

#include "features/department_members/providers/department_member_provider.hpp"
#include "features/department_members/ui/portal_department_members_interaction_support.hpp"
#include "features/departments/ui/portal_department_navigation_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>

#include <cstdlib>
#include <string>

class PortalRemoveDepartmentMemberAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto departmentId =
            std::atoi(context.requireRouteParam("id").c_str());
        const auto userId =
            std::atoi(context.requireRouteParam("userId").c_str());
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

        if (!context.requireService<PortalDepartmentMemberProvider>()
                ->removeMember(departmentId, userId)) {
            if (PortalDepartmentMembersInteractionSupport::isInteraction(
                context
            )) {
                return PortalDepartmentMembersInteractionSupport::render(
                    context,
                    departmentId,
                    returnUrl,
                    "member_not_found",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }
            return drogular::ActionResult::redirect(
                back + "&error=member_not_found"
            );
        }

        if (PortalDepartmentMembersInteractionSupport::isInteraction(
            context
        )) {
            return PortalDepartmentMembersInteractionSupport::render(
                context,
                departmentId,
                returnUrl,
                "",
                "member_removed"
            );
        }

        return drogular::ActionResult::redirect(
            back + "&success=member_removed"
        );
    }
};