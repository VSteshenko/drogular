#pragma once

#include "../ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

class PortalAdminPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "admin.title"
        );

        context.set("accessDenied", false);
        context.set("hasAdminAccess", false);

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        if (!drogular::PageAuthSupport::requireSessionValue(
                context,
                "role",
                "admin"
            )) {
            return;
            }

        context.set(
            "hasAdminAccess",
            true
        );
    }

    std::string templatePath() const override {
        return "admin.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};