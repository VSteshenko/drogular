#pragma once

#include "data/mini_store_store.hpp"
#include "features/staff/support/staff_support.hpp"

#include <drogular/action_handler.hpp>

class MiniStoreSaveProductAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        MiniStoreProduct product;
        product.id = context.form<int>("id").value_or(0);
        product.sku = context.form<std::string>("sku").value_or("");
        product.name = {
            {"en", context.form<std::string>("nameEn").value_or("")},
            {"de", context.form<std::string>("nameDe").value_or("")}
        };
        product.description = {
            {"en", context.form<std::string>("descriptionEn").value_or("")},
            {"de",context.form<std::string>("descriptionDe").value_or("")}
        };
        product.categoryId = context.form<int>("categoryId").value_or(0);
        product.priceCents =
            MiniStoreStaffSupport::cents(context.form<std::string>("price").value_or(""));
        product.stock = context.form<int>("stock").value_or(-1);
        product.active = context.form<std::string>("active").value_or("") == "on";

        if (!context.requireService<MiniStoreStore>()->saveProduct(std::move(product))) {
            return drogular::ActionResult::redirect("/staff/products?error=validation");
        }
        return drogular::ActionResult::redirect("/staff/products?success=saved");
    }
};