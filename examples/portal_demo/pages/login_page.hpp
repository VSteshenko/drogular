#pragma once

#include "../portal_page_support.hpp"

#include <drogular/page.hpp>

class PortalLoginPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        const auto request =
            context.request();

        const auto error =
            request != nullptr
                ? request->getParameter("error")
                : std::string("");

        context.set(
            "hasLoginError",
            !error.empty()
        );

        context.set(
            "loginError",
            error == "missing_credentials"
                ? std::string("Please enter username and password.")
                : error == "invalid_credentials"
                    ? std::string("Invalid username or password.")
                    : std::string("")
        );

        PortalPageSupport::apply(
            context,
            "login.title"
        );
    }

    std::string templatePath() const override
    {
        return "login.html";
    }

    std::string layoutPath() const override
    {
        return "layouts/main.html";
    }
};