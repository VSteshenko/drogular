#pragma once

#include "auth_store.hpp"
#include "auth_template_path.hpp"

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
        auto authStore =
            context.requireService<AuthStore>();

        const auto currentUser =
            authStore->currentUser.value();

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
        return authTemplatePath("dashboard.html");
    }
};