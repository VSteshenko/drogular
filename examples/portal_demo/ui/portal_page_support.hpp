#pragma once

#include "features/auth/support/portal_auth_support.hpp"

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
        context.set(
            "homeUrl",
            currentUser.has_value()
                ? std::string("/dashboard")
                : std::string("/login")
        );

        context.set(
            "locale",
            drogular::LocaleSupport::current(context)
        );

        const auto requestedTheme =
            context.cookie("dg_theme")
                .value_or("system");

        const auto theme =
            requestedTheme == "light" ||
            requestedTheme == "dark"
                ? requestedTheme
                : std::string("system");

        context.set("theme", theme);

        const auto currentPath =
            context.get<std::string>("currentPath")
                .value_or("/");

        const auto startsWith = [](
            const std::string& value,
            const std::string& prefix
        ) {
            return value.rfind(prefix, 0) == 0;
        };

        const auto rolesNavigationActive =
            startsWith(currentPath, "/roles");
        const auto projectTypesNavigationActive =
            startsWith(currentPath, "/project-types");
        const auto diagnosticsNavigationActive =
            startsWith(currentPath, "/__drogular");
        const auto adminNavigationActive =
            currentPath == "/admin" ||
            rolesNavigationActive ||
            projectTypesNavigationActive ||
            diagnosticsNavigationActive;

        context.set(
            "adminNavigationActive",
            adminNavigationActive
        );
        context.set(
            "rolesNavigationActive",
            rolesNavigationActive
        );
        context.set(
            "projectTypesNavigationActive",
            projectTypesNavigationActive
        );
        context.set(
            "diagnosticsNavigationActive",
            diagnosticsNavigationActive
        );
    }
};