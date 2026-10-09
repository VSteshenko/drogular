#pragma once

#include "data/mini_store_dataset.hpp"

#include <algorithm>
#include <mutex>
#include <optional>
#include <vector>

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

private:
    mutable std::mutex mutex_;
    MiniStoreDataset dataset_;
};