#pragma once

#include "domain/store_models.hpp"

#include <vector>

struct MiniStoreDataset {
    std::vector<MiniStoreCategory> categories;
    std::vector<MiniStoreProduct> products;
    std::vector<MiniStoreOrder> orders;
};

inline MiniStoreDataset createMiniStoreDemoDataset() {
    return {
        {
            {1, "Coffee & Tea", "coffee-tea"},
            {2, "Kitchen", "kitchen"},
            {3, "Accessories", "accessories"}
        },
        {
            {1, "COF-001", "House Blend Coffee", "Balanced whole-bean coffee for everyday brewing.", 1, 1290, 18, true},
            {2, "TEA-001", "Earl Grey Tea", "Black tea with bergamot in a reusable tin.", 1, 890, 24, true},
            {3, "KIT-001", "Glass Coffee Server", "Heat-resistant server for pour-over coffee.", 2, 2490, 7, true},
            {4, "ACC-001", "Canvas Market Bag", "Reusable canvas bag for everyday shopping.", 3, 1590, 12, true},
            {5, "KIT-002", "Digital Kitchen Scale", "Compact scale with one-gram precision.", 2, 2990, 4, true},
            {6, "ACC-002", "Travel Tumbler", "Insulated stainless-steel tumbler.", 3, 2190, 0, true}
        },
        {
            {
                1001,
                {"Alex Morgan", "alex@example.test"},
                {{1, "House Blend Coffee", 1290, 2}, {4, "Canvas Market Bag", 1590, 1}},
                4170,
                MiniStoreOrderStatus::New,
                "2026-10-08T09:30:00Z"
            },
            {
                1000,
                {"Sam Taylor", "sam@example.test"},
                {{3, "Glass Coffee Server", 2490, 1}},
                2490,
                MiniStoreOrderStatus::Processing,
                "2026-10-07T15:10:00Z"
            }
        }
    };
}