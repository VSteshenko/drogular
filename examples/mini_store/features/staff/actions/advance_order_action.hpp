#pragma once

#include "data/mini_store_store.hpp"

#include <drogular/action_handler.hpp>

class MiniStoreAdvanceOrderAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        const auto id = context.form<int>("id").value_or(0);

        if (!context.requireService<MiniStoreStore>()->advanceOrder(id)) {
            return drogular::ActionResult::redirect("/staff/orders/" + std::to_string(id) + "?error=transition");
        }
        return drogular::ActionResult::redirect("/staff/orders/" + std::to_string(id) + "?success=advanced");
    }
};