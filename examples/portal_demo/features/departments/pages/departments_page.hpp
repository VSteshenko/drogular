#pragma once

#include "ui/portal_page_support.hpp"
#include "features/departments/ui/portal_departments_browser_support.hpp"
#include "features/departments/ui/portal_department_create_form_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>


class PortalDepartmentsPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "departments.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        const auto request = context.request();

        PortalDepartmentCreateFormSupport::apply(
            context,
            request != nullptr
                ? request->getParameter("name")
                : std::string(),
            "",
            "",
            true,
            request != nullptr
                ? request->getParameter("error")
                : std::string(),
            request != nullptr
                ? request->getParameter("success")
                : std::string()
        );

        PortalDepartmentsBrowserSupport::apply(context);
    }

    std::string templatePath() const override {
        return "departments.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};