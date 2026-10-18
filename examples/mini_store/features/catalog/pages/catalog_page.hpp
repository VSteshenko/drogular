#pragma once

#include "data/mini_store_store.hpp"
#include "features/catalog/ui/catalog_query_parser.hpp"
#include "features/catalog/ui/catalog_support.hpp"
#include "support/storefront_page_support.hpp"
#include "ui/mini_store_frame_navigation_support.hpp"

#include <drogular/page.hpp>
#include <drogular/locale_support.hpp>

class MiniStoreCatalogPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        MiniStoreStorefrontPageSupport::apply(context, "catalog.title");

        const auto locale = drogular::LocaleSupport::current(context);
        const auto query = MiniStoreCatalogQueryParser::fromRequest(context.request());
        const auto store = context.requireService<MiniStoreStore>();
        const auto categories = store->categories();
        const auto products =
            MiniStoreCatalogSupport::filter(store->products(), categories, query, locale);

        Json::Value categoryItems(Json::arrayValue);
        for (const auto& category : categories) {
            Json::Value item;
            item["slug"] = category.slug;
            item["name"] = std::string(category.name.resolve(locale));
            item["selected"] = category.slug == query.category;

            categoryItems.append(item);
        }

        Json::Value productItems(Json::arrayValue);
        for (const auto& product : products) {
            Json::Value item;
            item["id"] = product.id;
            item["sku"] = product.sku;
            item["name"] = std::string(product.name.resolve(locale));
            item["description"] = std::string(product.description.resolve(locale));
            item["price"] = MiniStoreCatalogSupport::money(product.priceCents);
            item["stock"] = product.stock;
            item["detailsUrl"] = MiniStoreCatalogSupport::productUrl(product.id, query);
            item["detailsFragmentUrl"] =
                MiniStoreFrameNavigationSupport::storefrontFragmentUrl(item["detailsUrl"].asString());

            productItems.append(item);
        }

        context.set("categories", categoryItems);
        context.set("products", productItems);
        context.set("languageRedirect", MiniStoreCatalogSupport::catalogUrl(query));
        context.set("catalogHistoryUrl", MiniStoreCatalogSupport::catalogUrl(query));
        context.set("catalogSearch", query.search);
        context.set("catalogCategory", query.category);
        context.set("catalogSort", query.sort);
        context.set("sortName", query.sort == "name");
        context.set("sortPriceAsc", query.sort == "price-asc");
        context.set("sortPriceDesc", query.sort == "price-desc");
    }

    std::string templatePath() const override {
        return "catalog.html";
    }
    std::string layoutPath() const override {
        return "layouts/storefront.html";
    }
};