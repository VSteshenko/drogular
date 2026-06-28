#pragma once

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
        const auto pageTitle =
            context.translate(pageTitleKey);

        drogular::PageSupport::apply(
            context,
            pageTitle
        );

        context.set("appTitle",
            context.translate("app.title"));
        context.set("navDashboard",
            context.translate("nav.dashboard"));
        context.set("navUsers",
            context.translate("nav.users"));
        context.set("navAdmin",
            context.translate("nav.admin"));
        context.set("navLogin",
            context.translate("nav.login"));
        context.set("loginUsername",
            context.translate("login.username"));
        context.set("loginPassword",
            context.translate("login.password"));
        context.set("loginSubmit",
            context.translate("login.submit"));
        context.set("navLogout",
            context.translate("nav.logout"));
        context.set("navProjects",
            context.translate("nav.projects"));

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
            context.translate("users.username"));
        context.set("usersPassword",
            context.translate("users.password"));
        context.set("usersRole",
            context.translate("users.role"));
        context.set("usersCreate",
            context.translate("users.create"));
        context.set("authLoginRequiredTitle",
            context.translate("auth.login_required.title"));
        context.set("authLoginRequiredMessage",
            context.translate("auth.login_required.message"));
        context.set("authAccessDeniedTitle",
            context.translate("auth.access_denied.title"));
        context.set("authAccessDeniedMessage",
            context.translate("auth.access_denied.message"));
        context.set("headerSignedInAs",
            context.translate("header.signed_in_as"));
        context.set("offlineMessage",
            context.translate("offline.message"));
        context.set("offlineHint",
            context.translate("offline.hint"));

        context.set("projectsTitleLabel",
            context.translate("projects.title.label"));
        context.set("projectsStatusLabel",
            context.translate("projects.status.label"));
        context.set("projectsCreate",
            context.translate("projects.create"));
        context.set("projectStatusActive",
            context.translate("projects.status.active"));
        context.set("projectStatusPaused",
            context.translate("projects.status.paused"));
        context.set("projectStatusDone",
            context.translate("projects.status.done"));
    }
};