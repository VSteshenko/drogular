#pragma once

#include <drogular/action_handler.hpp>

class PortalLanguageAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override
    {
        const auto language =
            context.requireForm<std::string>(
                "language"
            );

        const auto normalized =
            language == "de"
                ? "de"
                : "en";

        return drogular::ActionResult::redirect(
            "/dashboard"
        ).cookie(
            "lang",
            normalized
        );
    }
};