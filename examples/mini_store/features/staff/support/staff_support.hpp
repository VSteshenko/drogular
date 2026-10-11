#pragma once

#include "data/mini_store_store.hpp"

#include <drogular/render_context.hpp>

#include <cstdlib>
#include <string>

class MiniStoreStaffSupport {
public:
    static void applyCategories(
        drogular::RenderContext& context,
        const std::vector<MiniStoreCategory>& categories,
        int selected = 0
    ) {
        Json::Value values(Json::arrayValue);
        for (const auto& category : categories) {
            Json::Value value;
            value["id"] = category.id;
            value["name"] = std::string(category.name.resolve("en"));
            value["selected"] = category.id == selected; values.append(value);
        }
        context.set("categories", values);
    }

    static std::int64_t cents(std::string value) {
        try {
            return static_cast<std::int64_t>(std::stod(value) * 100.0 + 0.5);
        } catch (...) {
            return -1;
        }
    }
};