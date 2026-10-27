#pragma once

#include "data/mini_store_store.hpp"

#include <drogular/page.hpp>
#include "support/storefront_page_support.hpp"
#include <drogular/render_context.hpp>

class MiniStoreStorefrontHomePage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        MiniStoreStorefrontPageSupport::apply(context, "home.title");
        context.set("pageClass", std::string(" mini-store-page-home"));
        const auto store = context.requireService<MiniStoreStore>();
        const auto products = store->products();
        const auto categories = store->categories();
        context.set("productCount", static_cast<Json::UInt64>(products.size()));
        context.set("categoryCount", static_cast<Json::UInt64>(categories.size()));
    }

    std::string templatePath() const override {
        return "storefront_home.html";
    }
    std::string layoutPath() const override {
        return "layouts/storefront.html";
    }
};