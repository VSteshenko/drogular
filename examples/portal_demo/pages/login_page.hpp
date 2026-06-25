#pragma once

#include "../portal_page_support.hpp"
#include "../portal_error_translator.hpp"

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

        context.set(
            "loginError",
            PortalErrorTranslator::loginError(
                locale,
                error
            )
        );

        context.set(
            "alertMessage",
            PortalErrorTranslator::loginError(
                locale,
                error
            )
        );
    }

    std::string templatePath() const override {
        return "login.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};