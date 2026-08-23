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
            "locale",
            drogular::LocaleSupport::current(context)
        );
    }
};