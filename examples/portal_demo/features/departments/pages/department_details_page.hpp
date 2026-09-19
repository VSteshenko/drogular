#pragma once

#include "ui/portal_page_support.hpp"
#include "features/departments/providers/department_provider.hpp"
#include "features/departments/ui/portal_department_navigation_support.hpp"
#include "features/department_members/ui/portal_department_members_support.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>

class PortalDepartmentDetailsPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "departments.details.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        const auto id =
            std::atoi(context.requireRouteParam("id").c_str());

        const auto department =
            context.requireService<
                PortalDepartmentProvider
            >()->findById(id);

        context.set("departmentNotFound", !department.has_value());

        const auto request = context.request();
        const auto returnUrl =
            PortalDepartmentNavigationSupport::departmentsReturnUrl(
                request != nullptr
                    ? request->getParameter("returnUrl")
                    : std::string("")
            );
        context.set("departmentsBackUrl", returnUrl);
        context.set(
            "departmentEditUrl",
            PortalDepartmentNavigationSupport::editUrl(id, returnUrl)
        );

        if (!department) {
            return;
        }

        const auto users =
            context.requireService<PortalUserProvider>()->all();

        const auto memberships =
            context.requireService<
                PortalDepartmentMemberProvider
            >()->membersOfDepartment(id);

        const auto userById =
            [&users](int userId) -> std::optional<PortalUser> {
            for (const auto& user : users) {
                if (user.id == userId) {
                    return user;
                }
            }
            return std::nullopt;
        };

        context.set("departmentId", department->id);
        context.set("departmentName", department->name);
        context.set("departmentDescription", department->description);
        context.set("departmentActive", department->isActive);
        const auto manager = userById(department->managerId);
        context.set(
            "departmentManager",
            manager
            ? manager->username
            : "#" + std::to_string(department->managerId)
        );

        const auto error =
            request != nullptr
                ? request->getParameter("error")
                : std::string("");
        const auto success =
            request != nullptr
                ? request->getParameter("success")
                : std::string("");

        PortalDepartmentMembersSupport::apply(
            context,
            id,
            returnUrl,
            error,
            success
        );
    }

    std::string templatePath() const override {
        return "department_details.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};