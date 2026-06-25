#pragma once

#include <drogular/action_handler.hpp>

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
            redirect.starts_with("/")
                ? redirect
                : std::string("/dashboard");

        return drogular::ActionResult::redirect(
            safeRedirect
        ).cookie(
            "lang",
            normalized
        );
    }
};