#pragma once

#include "data/mini_store_store.hpp"
#include "features/staff/support/order_support.hpp"
#include "features/staff/support/staff_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_support.hpp>

#include <algorithm>

class MiniStoreStaffOrdersPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        const auto request = context.request();
        const auto filter = request ? request->getParameter("status") : std::string{};

        auto orders = context.requireService<MiniStoreStore>()->orders();
        std::sort(orders.begin(), orders.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.createdAt > rhs.createdAt;
        });

        const auto selectedFilter = filter.empty() ? std::string{"all"} : filter;
        std::string historyUrl = MiniStoreOrderSupport::safeOrderReturnUrl({});
        if (selectedFilter != "all") {
            historyUrl += "?status=";
            historyUrl += selectedFilter;
        }

        MiniStoreStaffPageSupport::apply(
            context,
            "staff.orders.title",
            historyUrl
        );

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
            row["status"] =
                context.translate("staff.order.status." + MiniStoreOrderSupport::statusKey(order.status));
            row["createdAt"] = MiniStoreOrderSupport::displayTimestamp(order.createdAt);
            row["statusNew"] = order.status == MiniStoreOrderStatus::New;
            row["statusProcessing"] = order.status == MiniStoreOrderStatus::Processing;
            row["statusCompleted"] = order.status == MiniStoreOrderStatus::Completed;
            row["url"] = MiniStoreOrderSupport::orderUrl(order.id, historyUrl);
            row["fragmentUrl"] =
                MiniStoreFrameNavigationSupport::staffOrderFragmentUrl(row["url"].asString());

            rows.append(row);
        }

        context.set("orders", rows);
        context.set("filter", selectedFilter);
        context.set("historyUrl", historyUrl);
        context.set("filterAll", selectedFilter == "all");
        context.set("filterNew", selectedFilter == "new");
        context.set("filterProcessing", selectedFilter == "processing");
        context.set("filterCompleted", selectedFilter == "completed");
    }

    std::string templatePath() const override {
        return "staff_orders.html";
    }
    std::string layoutPath() const override {
        return "layouts/staff.html";
    }
};