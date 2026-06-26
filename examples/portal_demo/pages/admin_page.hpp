#pragma once

#include "../auth/portal_auth_support.hpp"
#include "../ui/portal_page_support.hpp"

#include <drogular/page.hpp>

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

        const auto currentUser =
            PortalAuthSupport::currentUser(context);

        context.set(
            "loginRequired",
            !currentUser.has_value()
        );

        context.set(
            "accessDenied",
            currentUser.has_value() &&
            currentUser->role != "admin"
        );

        context.set(
            "hasAdminAccess",
            currentUser.has_value() &&
            currentUser->role == "admin"
        );
    }

    std::string templatePath() const override {
        return "admin.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};