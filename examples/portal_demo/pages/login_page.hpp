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
        PortalPageSupport::apply(
            context,
            "login.title"
        );

        const auto locale =
            PortalLocale::fromRenderContext(context);

        PortalTranslations translations;

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

        std::string loginError;

        if (error == "missing_credentials") {
            loginError = translations.get(
                locale,
                "login.error.missing_credentials"
            );
        } else if (error == "invalid_credentials") {
            loginError = translations.get(
                locale,
                "login.error.invalid_credentials"
            );
        }

        context.set("loginError", loginError);
    }

    std::string templatePath() const override {
        return "login.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};