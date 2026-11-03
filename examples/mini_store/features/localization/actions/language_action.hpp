#pragma once

#include <drogular/action_handler.hpp>
#include <drogular/navigation_context.hpp>

class MiniStoreLanguageAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        const auto language = context.requireForm<std::string>("language");
        const auto normalized = language == "de" ? "de" : "en";
        const auto redirect = context.form<std::string>("redirect").value_or("/");
        const auto safe = drogular::NavigationContext::resolve(redirect, "/");

        return drogular::ActionResult::redirect(safe).cookie("lang", normalized);
    }
};