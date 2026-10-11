#pragma once

#include "data/mini_store_store.hpp"
#include "features/cart/support/cart_support.hpp"
#include "features/catalog/ui/catalog_support.hpp"
#include "support/storefront_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/locale_support.hpp>

class MiniStoreCheckoutPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        MiniStoreStorefrontPageSupport::apply(context, "checkout.title");
        context.set("languageRedirect", "/checkout");

        const auto cart =
            MiniStoreCartSupport::read(MiniStoreCartSupport::existing(context));
        const auto store = context.requireService<MiniStoreStore>();
        const auto locale = drogular::LocaleSupport::current(context);

        Json::Value rows(Json::arrayValue);
        std::int64_t total = 0;
        for (const auto& ci:cart) {
            const auto product = store->product(ci.productId);
            if(!product || !product->active) {
                continue;
            }

            Json::Value row;
            row["name"] = std::string(product->name.resolve(locale));
            row["quantity"] = ci.quantity;
            row["lineTotal"] =
                MiniStoreCatalogSupport::money(product->priceCents * ci.quantity);
            rows.append(row);

            total += product->priceCents * ci.quantity;
        }

        context.set("cartItems", rows);
        context.set("cartEmpty", rows.empty());
        context.set("cartTotal", MiniStoreCatalogSupport::money(total));

        const auto req = context.request();
        const auto error = req ? req->getParameter("error") : std::string{};
        context.set("customerError", error == "customer");
        context.set("stockError", error == "stock");
    }

    std::string templatePath() const override {
        return "checkout.html";
    }
    std::string layoutPath() const override {
        return "layouts/storefront.html";
    }
};