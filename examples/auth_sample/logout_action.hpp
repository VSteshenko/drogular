#pragma once

#include "auth_store.hpp"

#include <drogular/action_handler.hpp>

class LogoutAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override
    {
        auto authStore =
            context.requireService<AuthStore>();

        authStore->currentUser.set(
            std::nullopt
        );

        return drogular::ActionResult::redirect(
            "/login"
        );
    }
};
