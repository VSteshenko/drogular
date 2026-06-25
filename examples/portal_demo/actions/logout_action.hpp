#pragma once

#include <drogular/action_handler.hpp>
#include <drogular/session_store.hpp>

class PortalLogoutAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto sessionId =
            context.cookie("session_id");

        if (sessionId.has_value()) {
            auto store =
                context.requireService<drogular::SessionStore>();

            store->remove(*sessionId);
        }

        return drogular::ActionResult::redirect(
            "/login"
        ).cookie(
            "session_id",
            ""
        );
    }
};