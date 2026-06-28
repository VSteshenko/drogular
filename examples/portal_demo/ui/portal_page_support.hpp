#pragma once

#include "../auth/portal_auth_support.hpp"

#include <drogular/page_support.hpp>
#include <drogular/render_context.hpp>
#include <drogular/translation_support.hpp>
#include <drogular/locale_support.hpp>

#include <string>

class PortalPageSupport {
public:
    static void apply(
        drogular::RenderContext& context,
        const std::string& pageTitleKey
    ) {
        const auto pageTitle =
            drogular::TranslationSupport::translate(context, pageTitleKey);

        drogular::PageSupport::apply(
            context,
            pageTitle
        );

        context.set("appTitle",
            drogular::TranslationSupport::translate(context, "app.title"));
        context.set("navDashboard",
            drogular::TranslationSupport::translate(context, "nav.dashboard"));
        context.set("navUsers",
            drogular::TranslationSupport::translate(context, "nav.users"));
        context.set("navAdmin",
            drogular::TranslationSupport::translate(context, "nav.admin"));
        context.set("navLogin",
            drogular::TranslationSupport::translate(context, "nav.login"));
        context.set("loginUsername",
            drogular::TranslationSupport::translate(context, "login.username"));
        context.set("loginPassword",
            drogular::TranslationSupport::translate(context, "login.password"));
        context.set("loginSubmit",
            drogular::TranslationSupport::translate(context, "login.submit"));
        context.set("navLogout",
            drogular::TranslationSupport::translate(context, "nav.logout"));
        context.set("navProjects",
            drogular::TranslationSupport::translate(context, "nav.projects"));

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

        const auto locale =
            drogular::LocaleSupport::current(context);

        context.set("isGerman", locale == "de");
        context.set("isEnglish", locale == "en");

        const auto request =
            context.request();

        context.set("usersUsername",
            drogular::TranslationSupport::translate(context, "users.username"));
        context.set("usersPassword",
            drogular::TranslationSupport::translate(context, "users.password"));
        context.set("usersRole",
            drogular::TranslationSupport::translate(context, "users.role"));
        context.set("usersCreate",
            drogular::TranslationSupport::translate(context, "users.create"));
        context.set("authLoginRequiredTitle",
            drogular::TranslationSupport::translate(context, "auth.login_required.title"));
        context.set("authLoginRequiredMessage",
            drogular::TranslationSupport::translate(context, "auth.login_required.message"));
        context.set("authAccessDeniedTitle",
            drogular::TranslationSupport::translate(context, "auth.access_denied.title"));
        context.set("authAccessDeniedMessage",
            drogular::TranslationSupport::translate(context, "auth.access_denied.message"));
        context.set("headerSignedInAs",
            drogular::TranslationSupport::translate(context, "header.signed_in_as"));
        context.set("offlineMessage",
            drogular::TranslationSupport::translate(context, "offline.message"));
        context.set("offlineHint",
            drogular::TranslationSupport::translate(context, "offline.hint"));

        context.set("projectsTitleLabel",
            drogular::TranslationSupport::translate(context, "projects.title.label"));
        context.set("projectsStatusLabel",
            drogular::TranslationSupport::translate(context, "projects.status.label"));
        context.set("projectsCreate",
            drogular::TranslationSupport::translate(context, "projects.create"));
        context.set("projectStatusActive",
            drogular::TranslationSupport::translate(context, "projects.status.active"));
        context.set("projectStatusPaused",
            drogular::TranslationSupport::translate(context, "projects.status.paused"));
        context.set("projectStatusDone",
            drogular::TranslationSupport::translate(context, "projects.status.done"));
    }
};