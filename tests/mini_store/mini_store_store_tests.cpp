#include "data/mini_store_store.hpp"
#include "features/catalog/ui/catalog_support.hpp"

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

TEST(MiniStoreCatalogTests, FiltersByLocalizedNameAndCategory) {
    MiniStoreStore store;
    MiniStoreCatalogQuery query;
    query.search = "Kaffee";
    query.category = "coffee-tea";

    const auto products = MiniStoreCatalogSupport::filter(
        store.products(), store.categories(), query, "de"
    );

    ASSERT_EQ(products.size(), 1U);
    EXPECT_EQ(products.front().sku, "COF-001");
}

TEST(MiniStoreCatalogTests, SortsByPrice) {
    MiniStoreStore store;
    MiniStoreCatalogQuery query;
    query.sort = "price-desc";

    const auto products = MiniStoreCatalogSupport::filter(
        store.products(), store.categories(), query, "en"
    );

    ASSERT_FALSE(products.empty());
    EXPECT_EQ(products.front().sku, "KIT-002");
}

TEST(MiniStoreCatalogTests, PreservesCatalogStateInProductReturnUrl) {
    MiniStoreCatalogQuery query;
    query.search = "coffee server";
    query.category = "kitchen";
    query.sort = "price-asc";

    EXPECT_EQ(
        MiniStoreCatalogSupport::catalogUrl(query),
        "/catalog?q=coffee%20server&category=kitchen&sort=price-asc"
    );
    EXPECT_EQ(
        MiniStoreCatalogSupport::productUrl(3, query),
        "/products/3?returnUrl=%2Fcatalog%3Fq%3Dcoffee%2520server%26category%3Dkitchen%26sort%3Dprice-asc"
    );
}

TEST(MiniStoreCatalogTests, RejectsUnsafeReturnUrl) {
    EXPECT_EQ(MiniStoreCatalogSupport::safeCatalogReturnUrl("https://example.test"), "/catalog");
    EXPECT_EQ(MiniStoreCatalogSupport::safeCatalogReturnUrl("//example.test"), "/catalog");
    EXPECT_EQ(MiniStoreCatalogSupport::safeCatalogReturnUrl("/catalog?category=kitchen"), "/catalog?category=kitchen");
}