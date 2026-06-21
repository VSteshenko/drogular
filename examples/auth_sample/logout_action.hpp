#pragma once

#include <drogular/action_handler.hpp>
#include <drogular/session_store.hpp>

class LogoutAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override
    {
        auto sessionId =
            context.cookie("session_id");

        if (sessionId.has_value()) {
            auto sessionStore =
                context.requireService<drogular::SessionStore>();

            sessionStore->remove(*sessionId);
        }

        return drogular::ActionResult::redirect("/login")
            .cookie("session_id", "");
    }
};