#pragma once

#include <drogular/action_handler.hpp>
#include <drogular/navigation_context.hpp>

class PortalLanguageAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto language =
            context.requireForm<std::string>(
                "language"
            );

        const auto normalized =
            language == "de"
                ? "de"
                : "en";

        const auto redirect =
            context.form<std::string>("redirect")
                .value_or("/dashboard");

        const auto safeRedirect =
            drogular::NavigationContext::resolve(redirect, "/dashboard");

        return drogular::ActionResult::redirect(
            safeRedirect
        ).cookie(
            "lang",
            normalized
        );
    }
};