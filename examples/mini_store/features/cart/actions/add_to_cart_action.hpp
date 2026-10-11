#pragma once

#include "data/mini_store_store.hpp"
#include "features/cart/support/cart_support.hpp"

#include <drogular/action_handler.hpp>

class MiniStoreAddToCartAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        const auto productId = context.requireForm<int>("productId");
        const auto quantity =
            std::max(1, context.form<int>("quantity").value_or(1));
        const auto product =
            context.requireService<MiniStoreStore>()->product(productId);
        const auto returnUrl =
            MiniStoreCartSupport::safeReturnUrl(context.form<std::string>("returnUrl").value_or("/cart"));

        if (!product || !product->active || product->stock <= 0) {
            return drogular::ActionResult::redirect(returnUrl);
        }

        auto session = context.session();
        auto items = MiniStoreCartSupport::read(session);
        auto it = std::find_if(
            items.begin(),
            items.end(),
            [&](const auto& item) {
                return item.productId == productId;
            }
        );
        const auto requested = std::min(quantity, product->stock);

        if (it == items.end()) {
            items.push_back({productId, requested});
        }
        else {
            it->quantity = std::min(it->quantity + requested, product->stock);
        }

        MiniStoreCartSupport::write(session, items);
        return drogular::ActionResult::redirect(returnUrl)
            .cookie("session_id", session->get("_id").value());
    }
};