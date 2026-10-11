#pragma once

#include "data/mini_store_store.hpp"

#include <drogular/page.hpp>
#include <drogular/page_support.hpp>

#include <cstdlib>

class MiniStoreStaffCategoryFormPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        drogular::PageSupport::apply(context, "Category");

        auto store = context.requireService<MiniStoreStore>();
        int id = 0;
        auto raw = context.routeParam("id");
        if (raw) {
            id = std::atoi(raw->c_str());
        }

        auto category = id
            ? store->category(id)
            : std::optional<MiniStoreCategory>{};
        context.set("editing", category.has_value());
        context.set("id", id);
        context.set("slug", category ? category->slug : "");
        context.set("nameEn", category
            ? std::string(category->name.resolve("en"))
            : "");
        context.set("nameDe", category
            ? std::string(category->name.resolve("de"))
            : "");
    }

    std::string templatePath() const override {
        return "staff_category_form.html";
    }
    std::string layoutPath() const override {
        return "layouts/staff.html";
    }
};