#pragma once

#include "data/mini_store_store.hpp"
#include "features/cart/support/cart_support.hpp"
#include "features/cart/pages/cart_page.hpp"
#include "ui/mini_store_page_fragment_action.hpp"

#include <drogular/action_renderer.hpp>
#include <drogular/action_handler.hpp>

class MiniStoreUpdateCartAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        const auto productId = context.requireForm<int>("productId");
        const auto quantity = context.form<int>("quantity").value_or(0);
        auto session = context.existingSession();
        if (!session) {
            return drogular::ActionResult::redirect("/cart");
        }

        auto items = MiniStoreCartSupport::read(session);
        auto it = std::find_if(
            items.begin(),
            items.end(),
            [&](const auto& item) {
                return item.productId == productId;
            });
        if (it != items.end()) {
            const auto product =
                context.requireService<MiniStoreStore>()->product(productId);

            if (quantity <= 0 || !product || !product->active) {
                items.erase(it);
            }
            else {
                it->quantity = std::min(quantity, product->stock);
            }
        }

        MiniStoreCartSupport::write(session, items);
        if (context.isInteraction()) {
            return drogular::ActionRenderer::render<
                MiniStoreStorefrontPageFragmentComponent<MiniStoreCartPage>
            >(
                context,
                [](drogular::RenderContext& renderContext) {
                    MiniStoreCartPage::apply(renderContext);
                }
            );
        }
        return drogular::ActionResult::redirect("/cart");
    }
};