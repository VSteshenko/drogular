#pragma once

#include "../localization/portal_translations.hpp"
#include "../auth/portal_auth_support.hpp"

#include <drogular/page_support.hpp>
#include <drogular/render_context.hpp>
#include <drogular/locale_support.hpp>

#include <string>

class PortalPageSupport {
public:
    static void apply(
        drogular::RenderContext& context,
        const std::string& pageTitleKey
    ) {
        const auto locale =
            drogular::LocaleSupport::current(context);

        auto translations =
            context.requireService<PortalTranslations>();

        const auto pageTitle = translations->get(locale, pageTitleKey);

        drogular::PageSupport::apply(
            context,
            pageTitle
        );

        context.set("locale", locale);

        context.set("appTitle", translations->get(locale, "app.title"));
        context.set("navDashboard", translations->get(locale, "nav.dashboard"));
        context.set("navUsers", translations->get(locale, "nav.users"));
        context.set("navAdmin", translations->get(locale, "nav.admin"));
        context.set("navLogin", translations->get(locale, "nav.login"));
        context.set("loginUsername", translations->get(locale, "login.username"));
        context.set("loginPassword", translations->get(locale, "login.password"));
        context.set("loginSubmit", translations->get(locale, "login.submit"));
        context.set("navLogout", translations->get(locale, "nav.logout"));
        context.set("navProjects", translations->get(locale, "nav.projects"));

        const auto currentUser =
            PortalAuthSupport::currentUser(context);

        context.set("isAuthenticated", currentUser.has_value());
        context.set("currentUsername", currentUser.has_value()
            ? currentUser->username
            : std::string("")
        );
        context.set("currentRole", currentUser.has_value()
            ? currentUser->role
            : std::string("")
        );
        context.set("isAdmin", currentUser.has_value() && currentUser->role == "admin");
        context.set("isLoginPage", pageTitleKey == "login.title");
        context.set("isGerman", locale == "de");
        context.set("isEnglish", locale == "en");

        const auto request =
            context.request();

        context.set("usersUsername", translations->get(locale, "users.username"));
        context.set("usersPassword", translations->get(locale, "users.password"));
        context.set("usersRole", translations->get(locale, "users.role"));
        context.set("usersCreate", translations->get(locale, "users.create"));
        context.set("authLoginRequiredTitle", translations->get(locale, "auth.login_required.title"));
        context.set("authLoginRequiredMessage", translations->get(locale, "auth.login_required.message"));
        context.set("authAccessDeniedTitle", translations->get(locale, "auth.access_denied.title"));
        context.set("authAccessDeniedMessage", translations->get(locale, "auth.access_denied.message"));
        context.set("headerSignedInAs", translations->get(locale, "header.signed_in_as"));
        context.set("offlineMessage", translations->get(locale, "offline.message"));
        context.set("offlineHint", translations->get(locale, "offline.hint"));

        context.set("projectsTitleLabel", translations->get(locale, "projects.title.label"));
        context.set("projectsStatusLabel", translations->get(locale, "projects.status.label"));
        context.set("projectsCreate", translations->get(locale, "projects.create"));
        context.set("projectStatusActive", translations->get(locale, "projects.status.active"));
        context.set("projectStatusPaused", translations->get(locale, "projects.status.paused"));
        context.set("projectStatusDone", translations->get(locale, "projects.status.done"));
    }
};