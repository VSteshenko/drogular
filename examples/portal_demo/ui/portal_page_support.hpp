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

        context.setTranslations({

            {"appTitle", "app.title"},

            {"navDashboard", "nav.dashboard"},
            {"navUsers", "nav.users"},
            {"navProjects", "nav.projects"},
            {"navAdmin", "nav.admin"},
            {"navLogin", "nav.login"},
            {"navLogout", "nav.logout"},

            {"loginUsername", "login.username"},
            {"loginPassword", "login.password"},
            {"loginSubmit", "login.submit"},

            {"usersUsername", "users.username"},
            {"usersPassword", "users.password"},
            {"usersRole", "users.role"},
            {"usersCreate", "users.create"},

            {"projectsTitleLabel", "projects.title.label"},
            {"projectsStatusLabel", "projects.status.label"},
            {"projectsCreate", "projects.create"},
            {"projectStatusActive", "projects.status.active"},
            {"projectStatusPaused", "projects.status.paused"},
            {"projectStatusDone", "projects.status.done"},
            {"projectNotFoundTitle", "projects.not_found.title"},
            {"projectNotFoundMessage", "projects.not_found.message"},
            {"projectsBack", "projects.back"},
            {"projectsUpdate", "projects.update"},
            {"projectsBackToDetails", "projects.back_to_details"},

            {"authLoginRequiredTitle", "auth.login_required.title"},
            {"authLoginRequiredMessage", "auth.login_required.message"},
            {"authAccessDeniedTitle", "auth.access_denied.title"},
            {"authAccessDeniedMessage", "auth.access_denied.message"},

            {"headerSignedInAs", "header.signed_in_as"},

            {"offlineMessage", "offline.message"},
            {"offlineHint", "offline.hint"}
        });
    }
};