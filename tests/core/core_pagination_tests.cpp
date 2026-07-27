#include <drogular/pagination.hpp>
#include <drogular/pagination_model.hpp>
#include <drogular/json_conversion.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(PaginationTests, PaginatesVectorValues) {
    const std::vector<int> values{1, 2, 3, 4, 5};

    const auto result =
        drogular::paginate(values, 2, 2);

    EXPECT_EQ(result.page, 2);
    EXPECT_EQ(result.pageSize, 2);
    EXPECT_EQ(result.totalItems, 5);
    EXPECT_EQ(result.totalPages, 3);
    EXPECT_EQ(result.items, (std::vector<int>{3, 4}));
    EXPECT_TRUE(result.hasPrevious());
    EXPECT_TRUE(result.hasNext());
}

TEST(PaginationTests, KeepsAtLeastOnePageForEmptyValues) {
    const auto result = drogular::paginate(
        std::vector<int>{},
        1,
        10
    );

    EXPECT_EQ(result.page, 1);
    EXPECT_EQ(result.totalPages, 1);
    EXPECT_TRUE(result.items.empty());
}

TEST(PaginationTests, NormalizesPageSizeAndClampsRequestedPage) {
    const std::vector<int> values{1, 2, 3};

    const auto result = drogular::paginate(values, 99, 2);

    EXPECT_EQ(result.page, 2);
    EXPECT_EQ(result.totalPages, 2);
    EXPECT_EQ(result.items, (std::vector<int>{3}));
}

TEST(PaginationTests, BuildsNavigationModel) {
    const auto model = drogular::makePaginationModel(
        2,
        3,
        [](int page) {
            return "/items?page=" + std::to_string(page);
        }
    );

    EXPECT_TRUE(model.visible);
    EXPECT_TRUE(model.hasPrevious);
    EXPECT_TRUE(model.hasNext);
    EXPECT_EQ(model.previousUrl, "/items?page=1");
    EXPECT_EQ(model.nextUrl, "/items?page=3");
    ASSERT_EQ(model.pages.size(), 3u);
    EXPECT_TRUE(model.pages[1].current);
}

TEST(PaginationTests, HidesSinglePageNavigation) {
    const auto model = drogular::makePaginationModel(
        1,
        1,
        [](int page) {
            return "/items?page=" + std::to_string(page);
        }
    );

    EXPECT_FALSE(model.visible);
    EXPECT_FALSE(model.hasPrevious);
    EXPECT_FALSE(model.hasNext);
    ASSERT_EQ(model.pages.size(), 1u);
}

TEST(PaginationTests, ConvertsNavigationModelToJson) {
    const auto model = drogular::makePaginationModel(
        2,
        3,
        [](int page) {
            return "/items?page=" + std::to_string(page);
        }
    );

    const auto json = drogular::toJsonValue(model);

    EXPECT_TRUE(json["visible"].asBool());
    EXPECT_EQ(json["previousUrl"].asString(), "/items?page=1");
    EXPECT_EQ(json["nextUrl"].asString(), "/items?page=3");
    ASSERT_EQ(json["pages"].size(), 3u);
    EXPECT_TRUE(json["pages"][1]["current"].asBool());
}