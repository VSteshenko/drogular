#pragma once

#include "data/mini_store_store.hpp"

#include <drogular/page.hpp>
#include <drogular/page_support.hpp>
#include <drogular/render_context.hpp>

#include <algorithm>

class MiniStoreStaffHomePage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        drogular::PageSupport::apply(context, "Staff dashboard");
        const auto store = context.requireService<MiniStoreStore>();
        const auto products = store->products();
        const auto orders = store->orders();
        const auto lowStock = std::count_if(
            products.begin(),
            products.end(),
            [](const auto& product) {
                return product.active && product.stock <= 5;
            });
        const auto newOrders = std::count_if(
            orders.begin(),
            orders.end(),
            [](const auto& order) {
                return order.status == MiniStoreOrderStatus::New;
            });
        const auto processingOrders = std::count_if(
            orders.begin(),
            orders.end(),
            [](const auto& order) {
                return order.status == MiniStoreOrderStatus::Processing;
            });
        const auto completedOrders = std::count_if(
            orders.begin(),
            orders.end(),
            [](const auto& order) {
                return order.status == MiniStoreOrderStatus::Completed;
            });
        context.set("productCount", static_cast<Json::UInt64>(products.size()));
        context.set("lowStockCount", static_cast<Json::UInt64>(lowStock));
        context.set("orderCount", static_cast<Json::UInt64>(orders.size()));
        context.set("newOrderCount", static_cast<Json::UInt64>(newOrders));
        context.set("processingOrderCount", static_cast<Json::UInt64>(processingOrders));
        context.set("completedOrderCount", static_cast<Json::UInt64>(completedOrders));
    }

    std::string templatePath() const override {
        return "staff_home.html";
    }
    std::string layoutPath() const override {
        return "layouts/staff.html";
    }
};