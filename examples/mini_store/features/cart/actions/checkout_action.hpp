#pragma once
#include "data/mini_store_store.hpp"
#include "features/cart/support/cart_support.hpp"
#include <drogular/action_handler.hpp>
#include <drogular/locale_support.hpp>

class MiniStoreCheckoutAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        const auto name = context.form<std::string>("name").value_or("");
        const auto email = context.form<std::string>("email").value_or("");
        if (name.empty() || email.empty() || email.find('@') == std::string::npos)
            return drogular::ActionResult::redirect("/checkout?error=customer");
        auto session = context.existingSession();
        const auto cart = MiniStoreCartSupport::read(session);
        if (cart.empty()) return drogular::ActionResult::redirect("/cart");
        const auto locale = context.cookie("lang").value_or("en") == "de" ? "de" : "en";
        const auto order = context.requireService<MiniStoreStore>()->placeOrder({name, email}, cart, locale);
        if (!order) return drogular::ActionResult::redirect("/checkout?error=stock");
        MiniStoreCartSupport::write(session, {});
        return drogular::ActionResult::redirect("/order-confirmation?id=" + std::to_string(order->id));
    }
};
