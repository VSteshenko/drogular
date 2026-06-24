#pragma once

#include "portal_locale.hpp"
#include "portal_translations.hpp"

#include <drogular/pwa_page_support.hpp>
#include <drogular/component.hpp>

#include <string>

class PortalPageSupport {
public:
    static void apply(
        drogular::RenderContext& context,
        const std::string& pageTitleKey
    ) {
        const auto locale =
            PortalLocale::fromRenderContext(context);

        PortalTranslations translations;

        drogular::PwaPageSupport::apply(context);

        context.set("locale", locale);

        context.set(
            "appTitle",
            translations.get(locale, "app.title")
        );

        context.set(
            "pageTitle",
            translations.get(locale, pageTitleKey)
        );

        context.set(
            "navDashboard",
            translations.get(locale, "nav.dashboard")
        );

        context.set(
            "navUsers",
            translations.get(locale, "nav.users")
        );

        context.set(
            "navAdmin",
            translations.get(locale, "nav.admin")
        );

        context.set(
            "navLogin",
            translations.get(locale, "nav.login")
        );

        context.set(
            "navLogout",
            translations.get(locale, "nav.logout")
        );
    }
};