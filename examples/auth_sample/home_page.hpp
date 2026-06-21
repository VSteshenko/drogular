#pragma once

#include "auth_session.hpp"

#include <drogular/page.hpp>

class HomePage final
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
    }

    std::string templatePath() const override {
        return "home.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};