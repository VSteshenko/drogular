#pragma once

#include "data/mini_store_store.hpp"
#include "features/staff/support/order_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_support.hpp>

#include <cstdlib>

class MiniStoreStaffOrderDetailsPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        drogular::PageSupport::apply(context, "Order");

        const auto request = context.request();
        const auto returnUrl =
            request ? request->getParameter("returnUrl") : std::string{};
        const auto orderBackUrl = MiniStoreOrderSupport::safeOrderReturnUrl(returnUrl);
        context.set("orderBackUrl", orderBackUrl);
        context.set(
            "orderBackFragmentUrl",
            MiniStoreFrameNavigationSupport::staffOrderFragmentUrl(orderBackUrl));

        const auto raw = context.routeParam("id");
        const auto id = raw ? std::atoi(raw->c_str()) : 0;
        const auto order = context.requireService<MiniStoreStore>()->order(id);
        context.set("found", order.has_value());
        context.set("id", id);
        if (!order) {
            return;
        }

        context.set("customer", order->customer.name);
        context.set("email", order->customer.email);
        context.set("status", MiniStoreOrderSupport::statusName(order->status));
        context.set("createdAt", order->createdAt);
        context.set("total", MiniStoreOrderSupport::money(order->totalCents));
        context.set("canAdvance", order->status != MiniStoreOrderStatus::Completed);
        context.set("nextStatus", order->status == MiniStoreOrderStatus::New ? "Processing" : "Completed");

        Json::Value items(Json::arrayValue);
        for (const auto& item : order->items) {
            Json::Value row;
            row["name"] = item.productName;
            row["quantity"] = item.quantity;
            row["unitPrice"] = MiniStoreOrderSupport::money(item.unitPriceCents);
            row["lineTotal"] = MiniStoreOrderSupport::money(item.unitPriceCents * item.quantity);
            items.append(row);
        }
        context.set("items", items);
    }

    std::string templatePath() const override {
        return "staff_order_details.html";
    }
    std::string layoutPath() const override {
        return "layouts/staff.html";
    }
};