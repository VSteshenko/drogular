#pragma once

#include "features/roles/ui/portal_role_edit_form_support.hpp"
#include "features/roles/providers/role_provider.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>
#include <string>

class PortalRoleEditPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "roles.edit_page.title"
        );

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

        const auto id =
            std::atoi(
                context.requireRouteParam("id").c_str()
            );

        auto roles =
            context.requireService<PortalRoleProvider>();

        const auto role =
            roles->findById(id);

        context.set("roleNotFound", !role.has_value());

        if (!role.has_value()) {
            return;
        }

        const auto request =
            context.request();

        const auto error =
            request != nullptr
                ? request->getParameter("error")
                : std::string("");

        const auto success =
            request != nullptr
                ? request->getParameter("success")
                : std::string("");

        PortalRoleEditFormSupport::apply(
            context,
            role->id,
            role->code,
            role->title,
            error,
            success
        );
    }

    std::string templatePath() const override {
        return "role_edit.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};