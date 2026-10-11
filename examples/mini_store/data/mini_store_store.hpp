#pragma once

#include "data/mini_store_dataset.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <optional>
#include <vector>

using Clock = std::chrono::system_clock;

class MiniStoreStore {
public:
    MiniStoreStore()
        : MiniStoreStore(createMiniStoreDemoDataset()) {}

    explicit MiniStoreStore(MiniStoreDataset dataset)
        : dataset_(std::move(dataset)) {}

    std::vector<MiniStoreCategory> categories() const {
        std::lock_guard lock(mutex_);
        return dataset_.categories;
    }

    std::vector<MiniStoreProduct> products() const {
        std::lock_guard lock(mutex_);
        return dataset_.products;
    }

    std::vector<MiniStoreOrder> orders() const {
        std::lock_guard lock(mutex_);
        return dataset_.orders;
    }

    std::optional<MiniStoreProduct> product(int id) const {
        std::lock_guard lock(mutex_);
        const auto it = std::find_if(
            dataset_.products.begin(), dataset_.products.end(),
            [id](const auto& product) { return product.id == id; }
        );
        return it == dataset_.products.end()
            ? std::nullopt
            : std::optional<MiniStoreProduct>(*it);
    }

    std::optional<MiniStoreOrder> placeOrder(
        MiniStoreCustomer customer,
        const std::vector<MiniStoreCartItem>& cart,
        const std::string& locale
    ) {
        std::lock_guard lock(mutex_);
        if (cart.empty()) {
            return std::nullopt;
        }

        MiniStoreOrder order;
        order.id = dataset_.orders.empty() ? 1000 : dataset_.orders.front().id + 1;
        order.customer = std::move(customer);
        order.status = MiniStoreOrderStatus::New;
        order.createdAt = currentTimestamp();

        for (const auto& cartItem : cart) {
            const auto it = std::find_if(
                dataset_.products.begin(),
                dataset_.products.end(),
                [&](const auto& product) {
                    return product.id == cartItem.productId;
                }
            );
            if (it == dataset_.products.end() ||
                !it->active ||
                cartItem.quantity <= 0 ||
                it->stock < cartItem.quantity
            ) {
                return std::nullopt;
            }

            order.items.push_back({
                it->id,
                std::string(it->name.resolve(locale)),
                it->priceCents,
                cartItem.quantity
            });
            order.totalCents += it->priceCents * cartItem.quantity;
        }
        for (const auto& cartItem : cart) {
            auto it = std::find_if(
                dataset_.products.begin(),
                dataset_.products.end(),
                [&](const auto& product) {
                    return product.id == cartItem.productId;
                });
            it->stock -= cartItem.quantity;
        }
        dataset_.orders.insert(dataset_.orders.begin(), order);

        return order;
    }

private:
    mutable std::mutex mutex_;
    MiniStoreDataset dataset_;

    static std::string currentTimestamp() {
        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);
        std::tm utc{};

#if defined(_WIN32)
        gmtime_s(&utc, &time);
#else
        gmtime_r(&time, &utc);
#endif

        std::ostringstream stream;
        stream << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");

        return stream.str();
    }
};