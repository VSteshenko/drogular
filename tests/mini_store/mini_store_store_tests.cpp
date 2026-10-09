#include "data/mini_store_store.hpp"

#include <gtest/gtest.h>

TEST(MiniStoreStoreTests, DemoDatasetIsSharedDomainFoundation) {
    MiniStoreStore store;

    EXPECT_EQ(store.categories().size(), 3U);
    EXPECT_EQ(store.products().size(), 6U);
    EXPECT_EQ(store.orders().size(), 2U);

    const auto product = store.product(1);
    ASSERT_TRUE(product.has_value());
    EXPECT_EQ(product->sku, "COF-001");
    EXPECT_EQ(product->priceCents, 1290);
}

TEST(MiniStoreStoreTests, OrderItemsKeepProductSnapshotData) {
    MiniStoreStore store;
    const auto orders = store.orders();

    ASSERT_FALSE(orders.empty());
    ASSERT_FALSE(orders.front().items.empty());
    EXPECT_EQ(orders.front().items.front().productName, "House Blend Coffee");
    EXPECT_EQ(orders.front().items.front().unitPriceCents, 1290);
}