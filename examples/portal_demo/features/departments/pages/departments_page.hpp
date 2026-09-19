#pragma once

#include "ui/portal_page_support.hpp"
#include "features/departments/ui/portal_departments_browser_support.hpp"
#include "features/localization/support/portal_error_translator.hpp"

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

        const auto request =
            context.request();

        const auto error = request != nullptr
            ? request->getParameter("error")
            : std::string();
        const auto success = request != nullptr
            ? request->getParameter("success")
            : std::string();
        const auto name = request != nullptr
            ? request->getParameter("name")
            : std::string();

        const auto departmentsError =
            PortalErrorTranslator::departmentsError(context, error);
        const auto departmentsSuccess =
            PortalErrorTranslator::departmentsSuccess(context, success);

        context.set("createDepartmentName", name);
        context.set("hasDepartmentsError", !departmentsError.empty());
        context.set("hasDepartmentsSuccess", !departmentsSuccess.empty());
        context.set(
            "alertMessage",
            !departmentsError.empty() ? departmentsError : departmentsSuccess
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