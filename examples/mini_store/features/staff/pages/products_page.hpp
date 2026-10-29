#pragma once

#include "data/mini_store_store.hpp"
#include "features/catalog/ui/catalog_support.hpp"
#include "features/staff/support/staff_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_support.hpp>

class MiniStoreStaffProductsPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        MiniStoreStaffPageSupport::apply(
            context,
            "staff.products.title",
            "/staff/products"
        );

        Json::Value rows(Json::arrayValue);
        for (const auto& product : context.requireService<MiniStoreStore>()->products()) {
            Json::Value value;
            value["id"] = product.id;
            value["sku"] = product.sku;
            value["nameEn"] = std::string(product.name.resolve("en"));
            value["nameDe"] = std::string(product.name.resolve("de"));
            value["price"] = MiniStoreCatalogSupport::money(product.priceCents);
            value["stock"] = std::to_string(product.stock);
            value["active"] = product.active;
            value["outOfStock"] = product.stock == 0;
            value["lowStock"] = product.stock > 0 && product.stock <= 5;
            value["inStock"] = product.stock > 5;
            value["stockClass"] = product.stock == 0
                ? "is-out"
                : (product.stock <= 5 ? "is-low" : "is-in");

            rows.append(value);
        }

        context.set("products",rows);
    }

    std::string templatePath() const override {
        return "staff_products.html";
    }
    std::string layoutPath() const override {
        return "layouts/staff.html";
    }
};