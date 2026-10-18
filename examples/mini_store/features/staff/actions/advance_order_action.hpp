#pragma once

#include "data/mini_store_store.hpp"
#include "features/staff/support/order_support.hpp"

#include <drogular/action_handler.hpp>

class MiniStoreAdvanceOrderAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        const auto id = context.form<int>("id").value_or(0);
        const auto returnUrl = MiniStoreOrderSupport::safeOrderReturnUrl(
            context.form<std::string>("returnUrl").value_or("")
        );
        const auto detailsUrl = MiniStoreOrderSupport::orderUrl(id, returnUrl);

        if (!context.requireService<MiniStoreStore>()->advanceOrder(id)) {
            return drogular::ActionResult::redirect(detailsUrl + "&error=transition");
        }
        return drogular::ActionResult::redirect(detailsUrl + "&success=advanced");
    }
};
