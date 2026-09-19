#pragma once

#include "ui/portal_page_support.hpp"
#include "features/departments/providers/department_provider.hpp"
#include "features/departments/ui/portal_department_edit_form_support.hpp"
#include "features/departments/ui/portal_department_navigation_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>
#include <string>

class PortalDepartmentEditPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        PortalPageSupport::apply(context, "departments.edit.title");
        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }
        if (!drogular::PageAuthSupport::requireSessionValue(context, "role", "admin")) {
            return;
        }

        const auto id = std::atoi(context.requireRouteParam("id").c_str());
        const auto department =
            context.requireService<PortalDepartmentProvider>()->findById(id);
        context.set("departmentNotFound", !department.has_value());
        if (!department) {
            return;
        }

        const auto request = context.request();
        const auto error = request
            ? request->getParameter("error")
            : std::string("");
        const auto returnUrl = PortalDepartmentNavigationSupport::departmentsReturnUrl(
            request ? request->getParameter("returnUrl") : std::string(""));
        PortalDepartmentEditFormSupport::apply(
            context, *department, department->name, department->description,
            department->managerId, department->isActive, error, "", returnUrl);
    }

    std::string templatePath() const override {
        return "department_edit.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};