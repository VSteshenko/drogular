#pragma once

#include "data/mini_store_store.hpp"
#include "features/catalog/ui/catalog_support.hpp"
#include "support/storefront_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/locale_support.hpp>

#include <cstdlib>

class MiniStoreProductDetailsPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        MiniStoreStorefrontPageSupport::apply(context, "product.title");

        const auto id = std::atoi(context.requireRouteParam("id").c_str());
        const auto request = context.request();
        const auto returnUrl =
            request ? request->getParameter("returnUrl") : std::string{};
        context.set("productBackUrl", MiniStoreCatalogSupport::safeCatalogReturnUrl(returnUrl));
        context.set("languageRedirect", "/products/" + std::to_string(id));

        const auto product = context.requireService<MiniStoreStore>()->product(id);
        context.set("productNotFound", !product.has_value());
        if (!product || !product->active) {
            context.set("productNotFound", true); return;
        }

        const auto locale = drogular::LocaleSupport::current(context);
        context.set("productName", std::string(product->name.resolve(locale)));
        context.set("productDescription", std::string(product->description.resolve(locale)));
        context.set("productSku", product->sku); context.set("productPrice", MiniStoreCatalogSupport::money(product->priceCents));
        context.set("productStock", product->stock);
        context.set("productId", product->id);
        context.set("productAddReturnUrl", "/cart");
    }

    std::string templatePath() const override {
        return "product_details.html";
    }
    std::string layoutPath() const override {
        return "layouts/storefront.html";
    }
};