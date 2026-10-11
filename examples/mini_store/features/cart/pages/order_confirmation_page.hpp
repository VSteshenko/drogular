#pragma once

#include "data/mini_store_store.hpp"
#include "features/catalog/ui/catalog_support.hpp"
#include "support/storefront_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/locale_support.hpp>

#include <cstdlib>

class MiniStoreOrderConfirmationPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        MiniStoreStorefrontPageSupport::apply(context, "order.confirmation.title");

        const auto req = context.request();
        const auto id = req ? std::atoi(req->getParameter("id").c_str()) : 0;
        context.set("languageRedirect", "/order-confirmation?id=" + std::to_string(id));

        const auto orders = context.requireService<MiniStoreStore>()->orders();
        const auto it = std::find_if(
            orders.begin(),
            orders.end(),
            [&](const auto&o) {
                return o.id==id;
            });
        context.set("orderFound", it != orders.end());
        if (it == orders.end()) {
            return;
        }

        context.set("orderId", it->id);
        context.set("customerName", it->customer.name);
        context.set("orderTotal", MiniStoreCatalogSupport::money(it->totalCents));
    }

    std::string templatePath() const override {
        return "order_confirmation.html";
    }
    std::string layoutPath() const override {
        return "layouts/storefront.html";
    }
};