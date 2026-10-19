#pragma once

#include "data/mini_store_store.hpp"
#include "features/staff/support/staff_support.hpp"
#include "features/staff/support/staff_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_support.hpp>

#include <cstdlib>

class MiniStoreStaffProductFormPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        auto store = context.requireService<MiniStoreStore>();
        int id = 0;
        auto raw = context.routeParam("id");
        if (raw) {
            id = std::atoi(raw->c_str());
        }

        MiniStoreStaffPageSupport::apply(
            context,
            "staff.product.title",
            id ? "/staff/products/" + std::to_string(id) + "/edit" : "/staff/products/new"
        );

        auto product= id
            ? store->product(id)
            : std::optional<MiniStoreProduct>{};
        context.set("editing", product.has_value());
        context.set("id", id);
        context.set("sku", product ? product->sku : "");
        context.set("nameEn", product
            ? std::string(product->name.resolve("en"))
            : "");
        context.set("nameDe", product
            ? std::string(product->name.resolve("de"))
            : "");
        context.set("descriptionEn", product
            ? std::string(product->description.resolve("en"))
            : "");
        context.set("descriptionDe", product
            ? std::string(product->description.resolve("de"))
            : "");
        context.set("price", product
            ? std::to_string(product->priceCents / 100) + "." +
                (product->priceCents % 100 < 10 ? "0"  : "") +
                std::to_string(product->priceCents % 100)
            : "");
        context.set("stock", product ? product->stock : 0);
        context.set("active", product ? product->active : true);

        MiniStoreStaffSupport::applyCategories(context, store->categories(), product ? product->categoryId : 0);
    }

    std::string templatePath() const override {
        return "staff_product_form.html";
    }
    std::string layoutPath() const override {
        return "layouts/staff.html";
    }
};