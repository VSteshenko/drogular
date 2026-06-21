#pragma once

#include "auth_session.hpp"

#include <drogular/page.hpp>
#include <drogular/component.hpp>

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
    }

    std::string templatePath() const override {
        return "dashboard.html";
    }
};
