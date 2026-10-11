#pragma once

#include "data/mini_store_store.hpp"

#include <drogular/render_context.hpp>
#include <drogular/session_store.hpp>

#include <json/json.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

class MiniStoreCartSupport {
public:
    static constexpr const char* sessionKey = "mini_store.cart";

    static std::vector<MiniStoreCartItem> read(const std::shared_ptr<drogular::Session>& session) {
        if (!session) {
            return {};
        }

        const auto value = session->get(sessionKey);
        if (!value || value->empty()) {
            return {};
        }

        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(
            value->data(),
            value->data() + value->size(),
            &root,
            &errors) || !root.isArray()
        ) {
            return {};
        }

        std::vector<MiniStoreCartItem> items;
        for (const auto& node : root) {
            if (!node.isObject()) {
                continue;
            }

            const auto productId = node.get("productId", 0).asInt();
            const auto quantity = node.get("quantity", 0).asInt();
            if (productId > 0 && quantity > 0) {
                items.push_back({productId, quantity});
            }
        }

        return items;
    }

    static void write(
        const std::shared_ptr<drogular::Session>& session,
        const std::vector<MiniStoreCartItem>& items
    ) {
        Json::Value root(Json::arrayValue);
        for (const auto& item : items) {
            if (item.productId <= 0 || item.quantity <= 0) {
                continue;
            }

            Json::Value node;
            node["productId"] = item.productId;
            node["quantity"] = item.quantity; root.append(node);
        }

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        session->set(sessionKey, Json::writeString(builder, root));
    }

    static std::shared_ptr<drogular::Session> existing(drogular::RenderContext& context) {
        const auto id = context.cookie("session_id");
        if (!id) {
            return nullptr;
        }

        const auto store = context.service<drogular::SessionStore>();
        return store ? store->get(*id) : nullptr;
    }

    static int count(const std::vector<MiniStoreCartItem>& items) {
        int result = 0;
        for (const auto& item : items) {
            result += item.quantity;
        }

        return result;
    }

    static std::string safeReturnUrl(const std::string& value, const std::string& fallback = "/cart") {
        return value.starts_with("/") && !value.starts_with("//") ? value : fallback;
    }
};