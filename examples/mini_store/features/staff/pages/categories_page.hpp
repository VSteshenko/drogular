#pragma once

#include "data/mini_store_store.hpp"
#include "features/staff/support/staff_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_support.hpp>

class MiniStoreStaffCategoriesPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        MiniStoreStaffPageSupport::apply(
            context,
            "staff.categories.title",
            "/staff/categories"
        );

        Json::Value rows(Json::arrayValue);
        for (const auto& category : context.requireService<MiniStoreStore>()->categories()) {
            Json::Value value;
            value["id"] = category.id;
            value["slug"] = category.slug;
            value["nameEn"] = std::string(category.name.resolve("en"));
            value["nameDe"] = std::string(category.name.resolve("de"));
            rows.append(value);
        }
        context.set("categoryRows", rows);
    }

    std::string templatePath() const override {
        return "staff_categories.html";
    }
    std::string layoutPath() const override {
        return "layouts/staff.html";
    }
};