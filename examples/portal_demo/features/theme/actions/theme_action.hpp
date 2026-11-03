#pragma once

#include <drogular/action_handler.hpp>
#include <drogular/navigation_context.hpp>

#include <string>

class PortalThemeAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto requested =
            context.requireForm<std::string>(
                "theme"
            );

        const auto theme =
            requested == "light" || requested == "dark"
                ? requested
                : std::string("system");

        const auto redirect =
            context.form<std::string>("redirect")
                .value_or("/dashboard");

        const auto safeRedirect =
            drogular::NavigationContext::resolve(redirect, "/dashboard");

        return drogular::ActionResult::redirect(
            safeRedirect
        ).cookie(
            "dg_theme",
            theme
        );
    }
};