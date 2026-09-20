#pragma once

#include "features/roles/ui/portal_role_create_form_support.hpp"
#include "features/roles/ui/portal_roles_list_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <string>

class PortalRolesPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "roles.page.title"
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

        context.set("hasAdminAccess", true);

        const auto request =
            context.request();

        PortalRoleCreateFormSupport::apply(
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

        PortalRolesListSupport::apply(context);
    }

    std::string templatePath() const override {
        return "roles.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};