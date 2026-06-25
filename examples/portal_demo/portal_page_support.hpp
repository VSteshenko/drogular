#pragma once

#include "portal_locale.hpp"
#include "portal_translations.hpp"
#include "portal_auth_support.hpp"

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
            "loginUsername",
            translations.get(locale, "login.username")
        );

        context.set(
            "loginPassword",
            translations.get(locale, "login.password")
        );

        context.set(
            "loginSubmit",
            translations.get(locale, "login.submit")
        );

        context.set(
            "navLogout",
            translations.get(locale, "nav.logout")
        );

        const auto currentUser =
            PortalAuthSupport::currentUser(context);

        context.set(
            "isAuthenticated",
            currentUser.has_value()
        );

        context.set(
            "currentUsername",
            currentUser.has_value()
                ? currentUser->username
                : std::string("")
        );

        context.set(
            "currentRole",
            currentUser.has_value()
                ? currentUser->role
                : std::string("")
        );

        context.set(
            "isAdmin",
            currentUser.has_value() && currentUser->role == "admin"
        );

        context.set(
            "isLoginPage",
            pageTitleKey == "login.title"
        );

        context.set(
            "isGerman",
            locale == "de"
        );

        context.set(
            "isEnglish",
            locale == "en"
        );

        const auto request =
            context.request();

        context.set(
            "currentPath",
            request != nullptr
                ? request->path()
                : std::string("/dashboard")
        );

        context.set(
            "usersUsername",
            translations.get(
                locale,
                "users.username"
            )
        );

        context.set(
            "usersPassword",
            translations.get(
                locale,
                "users.password"
            )
        );

        context.set(
            "usersRole",
            translations.get(
                locale,
                "users.role"
            )
        );

        context.set(
            "usersCreate",
            translations.get(
                locale,
                "users.create"
            )
        );
    }
};