#pragma once

#include "auth_session.hpp"

#include <drogular/page.hpp>
#include <drogular/render_context.hpp>

#include <string>

class DashboardPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        const auto currentUser =
            AuthSession::currentUser(context);

        context.set("pageTitle", std::string("Dashboard"));

        context.set(
            "authenticated",
            currentUser.has_value()
        );

        context.set(
            "username",
            currentUser.has_value()
                ? currentUser->username
                : std::string("")
        );

        context.set(
            "isAdmin",
            currentUser->role == "admin"
        );
    }

    std::string templatePath() const override {
        return "dashboard.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }

};
