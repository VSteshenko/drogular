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

TEST(MiniStoreLocalizedTextTests, ResolvesRequestedLocale) {
    const MiniStoreLocalizedText text{{"en", "Coffee"}, {"de", "Kaffee"}};

    EXPECT_EQ(text.resolve("de"), "Kaffee");
    EXPECT_EQ(text.resolve("en"), "Coffee");
}

TEST(MiniStoreLocalizedTextTests, FallsBackToDefaultLocale) {
    const MiniStoreLocalizedText text{{"en", "Coffee"}, {"de", "Kaffee"}};

    EXPECT_EQ(text.resolve("fr"), "Coffee");
    EXPECT_EQ(text.resolve("fr", "de"), "Kaffee");
}

TEST(MiniStoreLocalizedTextTests, ReturnsEmptyWhenRequestedAndFallbackLocalesAreMissing) {
    const MiniStoreLocalizedText text{{"de", "Kaffee"}};

    EXPECT_TRUE(text.resolve("fr").empty());
}

TEST(MiniStoreStoreTests, DemoCatalogContainsLocalizedProductAndCategoryContent) {
    MiniStoreStore store;

    const auto product = store.product(1);
    ASSERT_TRUE(product.has_value());
    EXPECT_EQ(product->name.resolve("en"), "House Blend Coffee");
    EXPECT_EQ(product->name.resolve("de"), "Hausmischung Kaffee");
    EXPECT_FALSE(product->description.resolve("de").empty());

    const auto categories = store.categories();
    ASSERT_FALSE(categories.empty());
    EXPECT_EQ(categories.front().name.resolve("en"), "Coffee & Tea");
    EXPECT_EQ(categories.front().name.resolve("de"), "Kaffee & Tee");
}