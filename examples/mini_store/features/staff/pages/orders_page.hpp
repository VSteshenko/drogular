#pragma once

#include "data/mini_store_store.hpp"
#include "features/staff/support/order_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_support.hpp>

#include <algorithm>

class MiniStoreStaffOrdersPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        drogular::PageSupport::apply(context, "Orders");
        const auto request = context.request();
        const auto filter = request ? request->getParameter("status") : std::string{};

        auto orders = context.requireService<MiniStoreStore>()->orders();
        std::sort(orders.begin(), orders.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.createdAt > rhs.createdAt;
        });

        Json::Value rows(Json::arrayValue);
        for (const auto& order : orders) {
            if (!MiniStoreOrderSupport::matches(order.status, filter)) {
                continue;
            }

            Json::Value row;
            row["id"] = order.id;
            row["customer"] = order.customer.name;
            row["email"] = order.customer.email;
            row["total"] = MiniStoreOrderSupport::money(order.totalCents);
            row["status"] = MiniStoreOrderSupport::statusName(order.status);
            row["createdAt"] = order.createdAt;
            rows.append(row);
        }
        context.set("orders", rows);
        context.set("filter", filter.empty() ? "all" : filter);
    }

    std::string templatePath() const override {
        return "staff_orders.html";
    }
    std::string layoutPath() const override {
        return "layouts/staff.html";
    }
};