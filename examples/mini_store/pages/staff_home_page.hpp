#pragma once

#include "data/mini_store_store.hpp"
#include "features/staff/support/staff_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/render_context.hpp>

#include <algorithm>

class MiniStoreStaffHomePage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        MiniStoreStaffPageSupport::apply(context, "staff.dashboard.title", "/staff");
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
        context.set("productCount", std::to_string(products.size()));
        context.set("lowStockCount", std::to_string(lowStock));
        context.set("orderCount", std::to_string(orders.size()));
        context.set("newOrderCount", std::to_string(newOrders));
        context.set("processingOrderCount", std::to_string(processingOrders));
        context.set("completedOrderCount", std::to_string(completedOrders));
    }

    std::string templatePath() const override {
        return "staff_home.html";
    }
    std::string layoutPath() const override {
        return "layouts/staff.html";
    }
};