#pragma once

#include "data/mini_store_store.hpp"
#include "features/cart/support/cart_support.hpp"
#include "features/catalog/ui/catalog_support.hpp"
#include "support/storefront_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/locale_support.hpp>

class MiniStoreCartPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        apply(context);
    }

    static void apply(drogular::RenderContext& context) {
        MiniStoreStorefrontPageSupport::apply(context, "cart.title");
        context.set("languageRedirect", "/cart");

        const auto locale = drogular::LocaleSupport::current(context);
        const auto store = context.requireService<MiniStoreStore>();
        const auto cart =
            MiniStoreCartSupport::read(MiniStoreCartSupport::existing(context));

        Json::Value rows(Json::arrayValue);
        std::int64_t total = 0;
        for (const auto& ci:cart) {
            const auto product = store->product(ci.productId);

            if (!product || !product->active) {
                continue;
            }

            Json::Value row;
            row["id"] = product->id;
            row["name"] = std::string(product->name.resolve(locale));
            row["price"] = MiniStoreCatalogSupport::money(product->priceCents);
            row["quantity"] = ci.quantity;
            row["stock"] = product->stock;
            row["lineTotal"] = MiniStoreCatalogSupport::money(product->priceCents * ci.quantity);
            rows.append(row);

            total += product->priceCents * ci.quantity;
        }

        context.set("cartItems", rows);
        context.set("cartEmpty", rows.empty());
        context.set("cartTotal", MiniStoreCatalogSupport::money(total));
        context.set("cartCount", MiniStoreCartSupport::count(cart));
    }

    std::string templatePath() const override {
        return "cart.html";
    }
    std::string layoutPath() const override {
        return "layouts/storefront.html";
    }
};