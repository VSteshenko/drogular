#pragma once

#include "auth_session.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>
#include <drogular/render_context.hpp>

#include <string>

class AdminPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        const auto currentUser =
            AuthSession::requireCurrentUser(context);

        if (!currentUser.has_value()) {
            return;
        }

        if (!drogular::PageAuthSupport::requireSessionValue(
                context,
                "role",
                "admin"
            )) {
            return;
        }

        context.set("pageTitle", std::string("Admin"));
        context.set("username", currentUser->username);
    }

    std::string templatePath() const override {
        return "admin.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};