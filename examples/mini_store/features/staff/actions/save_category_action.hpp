#pragma once

#include "data/mini_store_store.hpp"

#include <drogular/action_handler.hpp>

class MiniStoreSaveCategoryAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        MiniStoreCategory category;
        category.id = context.form<int>("id").value_or(0);
        category.slug = context.form<std::string>("slug").value_or("");
        category.name = {
            {"en", context.form<std::string>("nameEn").value_or("")},
            {"de", context.form<std::string>("nameDe").value_or("")}
        };

        if (!context.requireService<MiniStoreStore>()->saveCategory(std::move(category))) {
            return drogular::ActionResult::redirect("/staff/categories?error=validation");
        }
        return drogular::ActionResult::redirect("/staff/categories?success=saved");
    }
};