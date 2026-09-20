#pragma once

#include "features/project_types/ui/portal_project_type_create_form_support.hpp"
#include "features/project_types/ui/portal_project_types_list_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <string>

class PortalProjectTypesPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "project_types.page.title"
        );

        context.set("hasAdminAccess", false);

        if (!drogular::PageAuthSupport::requireAuthentication(
                context
            )) {
            return;
        }

        if (!drogular::PageAuthSupport::requireSessionValue(
                context,
                "role",
                "admin"
            )) {
            return;
        }

        const auto request =
            context.request();

        PortalProjectTypeCreateFormSupport::apply(
            context,
            request != nullptr
                ? request->getParameter("code")
                : std::string(""),
            request != nullptr
                ? request->getParameter("title")
                : std::string(""),
            request != nullptr
                ? request->getParameter("error")
                : std::string(""),
            request != nullptr
                ? request->getParameter("success")
                : std::string("")
        );

        context.set("hasAdminAccess", true);

        PortalProjectTypesListSupport::apply(context);
    }

    std::string templatePath() const override {
        return "project_types.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};