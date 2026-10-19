#pragma once

#include "support/storefront_page_support.hpp"

#include <drogular/page.hpp>

class MiniStoreOfflinePage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        MiniStoreStorefrontPageSupport::apply(context, "offline.title");
    }

    std::string templatePath() const override {
        return "offline.html";
    }

    std::string layoutPath() const override {
        return "layouts/storefront.html";
    }
};