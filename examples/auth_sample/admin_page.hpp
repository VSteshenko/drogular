#pragma once

#include "auth_service.hpp"
#include "auth_session.hpp"

#include <drogular/page.hpp>
#include <drogular/render_context.hpp>

#include <string>

class AdminPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        auto authService =
            context.requireService<AuthService>();

        const auto currentUser =
            AuthSession::currentUser(context);

        const auto isAdmin =
            currentUser.has_value() &&
            authService->hasRole(
                *currentUser,
                "admin"
            );

        context.set("pageTitle", std::string("Admin"));
        context.set("isAdmin", isAdmin);

        context.set(
            "username",
            currentUser.has_value()
                ? currentUser->username
                : std::string("")
        );
    }

    std::string templatePath() const override {
        return "admin.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};