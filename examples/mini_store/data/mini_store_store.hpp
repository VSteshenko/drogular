#pragma once

#include "data/mini_store_dataset.hpp"
#include "data/mini_store_time.hpp"

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

    std::optional<MiniStoreOrder> order(int id) const {
        std::lock_guard lock(mutex_);
        const auto it = std::find_if(
            dataset_.orders.begin(),
            dataset_.orders.end(),
            [id](const auto& order) {
                return order.id == id;
            }
        );
        return it == dataset_.orders.end()
            ? std::nullopt
            : std::optional<MiniStoreOrder>(*it);
    }

    bool advanceOrder(int id) {
        std::lock_guard lock(mutex_);
        const auto it = std::find_if(
            dataset_.orders.begin(),
            dataset_.orders.end(),
            [id](const auto& order) {
                return order.id == id;
            }
        );
        if (it == dataset_.orders.end()) {
            return false;
        }

        switch (it->status) {
            case MiniStoreOrderStatus::New:
                it->status = MiniStoreOrderStatus::Processing;
                return true;
            case MiniStoreOrderStatus::Processing:
                it->status = MiniStoreOrderStatus::Completed;
                return true;
            case MiniStoreOrderStatus::Completed:
                return false;
        }

        return false;
    }

    std::optional<MiniStoreProduct> product(int id) const {
        std::lock_guard lock(mutex_);
        const auto it = std::find_if(
            dataset_.products.begin(),
            dataset_.products.end(),
            [id](const auto& product) {
                return product.id == id;
            }
        );

        return it == dataset_.products.end()
            ? std::nullopt
            : std::optional<MiniStoreProduct>(*it);
    }

    std::optional<MiniStoreCategory> category(int id) const {
        std::lock_guard lock(mutex_);
        const auto it = std::find_if(
            dataset_.categories.begin(),
            dataset_.categories.end(),
            [id](const auto& category) {
                return category.id == id;
            }
        );

        return it == dataset_.categories.end()
            ? std::nullopt
            : std::optional<MiniStoreCategory>(*it);
    }

    bool saveCategory(MiniStoreCategory category) {
        std::lock_guard lock(mutex_);
        if (category.slug.empty() ||
            category.name.resolve("en").empty() ||
            category.name.resolve("de").empty()
        ) {
            return false;
        }

        const auto duplicate = std::find_if(
            dataset_.categories.begin(),
            dataset_.categories.end(),
            [&](const auto& item) {
                return item.slug == category.slug && item.id != category.id;
            }
        );
        if (duplicate != dataset_.categories.end()) {
            return false;
        }

        if (category.id == 0) {
            category.id = nextId(dataset_.categories);
            dataset_.categories.push_back(std::move(category));
            return true;
        }

        const auto it = std::find_if(
            dataset_.categories.begin(),
            dataset_.categories.end(),
            [&](const auto& item) {
                return item.id == category.id;
            }
        );
        if (it == dataset_.categories.end()) {
            return false;
        }

        *it = std::move(category);

        return true;
    }

    bool saveProduct(MiniStoreProduct product) {
        std::lock_guard lock(mutex_);

        if (product.sku.empty() ||
            product.priceCents < 0 ||
            product.stock < 0 ||
            product.name.resolve("en").empty() ||
            product.name.resolve("de").empty() ||
            product.description.resolve("en").empty() ||
            product.description.resolve("de").empty()
        ) {
            return false;
        }

        if (std::none_of(
            dataset_.categories.begin(),
            dataset_.categories.end(),
            [&](const auto& category) {
                return category.id == product.categoryId;
            })
        ) {
            return false;
        }

        const auto duplicate = std::find_if(
            dataset_.products.begin(),
            dataset_.products.end(),
            [&](const auto& item) {
                return item.sku == product.sku && item.id != product.id;
            }
        );
        if (duplicate != dataset_.products.end()) {
            return false;
        }

        if (product.id == 0) {
            product.id = nextId(dataset_.products);
            dataset_.products.push_back(std::move(product));
            return true;
        }

        const auto it = std::find_if(
            dataset_.products.begin(),
            dataset_.products.end(),
            [&](const auto& item) {
                return item.id == product.id;
            }
        );
        if (it == dataset_.products.end()) {
            return false;
        }

        *it = std::move(product);

        return true;
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
        order.createdAt = mini_store_time::now();

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

    template <typename T>
    static int nextId(const std::vector<T>& items) {
        int result = 1;
        for (const auto& item : items) {
            result = std::max(result, item.id + 1);
        }

        return result;
    }
};