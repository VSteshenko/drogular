#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class MiniStoreOrderStatus {
    New,
    Processing,
    Completed
};

struct MiniStoreCategory {
    int id = 0;
    std::string name;
    std::string slug;
};

struct MiniStoreProduct {
    int id = 0;
    std::string sku;
    std::string name;
    std::string description;
    int categoryId = 0;
    std::int64_t priceCents = 0;
    int stock = 0;
    bool active = true;
};

struct MiniStoreCartItem {
    int productId = 0;
    int quantity = 0;
};

struct MiniStoreOrderItem {
    int productId = 0;
    std::string productName;
    std::int64_t unitPriceCents = 0;
    int quantity = 0;
};

struct MiniStoreCustomer {
    std::string name;
    std::string email;
};

struct MiniStoreOrder {
    int id = 0;
    MiniStoreCustomer customer;
    std::vector<MiniStoreOrderItem> items;
    std::int64_t totalCents = 0;
    MiniStoreOrderStatus status = MiniStoreOrderStatus::New;
    std::string createdAt;
};