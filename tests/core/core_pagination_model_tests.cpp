#include <drogular/pagination.hpp>
#include <drogular/pagination_model.hpp>
#include <drogular/json_conversion.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(PaginationModelTests, BuildsLinksAndNavigationState) {
    const auto model = drogular::makePaginationModel(
        2,
        3,
        [](int page) {
            return "/projects?page=" + std::to_string(page);
        }
    );

    ASSERT_TRUE(model.visible);
    ASSERT_TRUE(model.hasPrevious);
    ASSERT_TRUE(model.hasNext);
    EXPECT_EQ(model.previousUrl, "/projects?page=1");
    EXPECT_EQ(model.nextUrl, "/projects?page=3");
    ASSERT_EQ(model.pages.size(), 3u);
    EXPECT_FALSE(model.pages[0].current);
    EXPECT_TRUE(model.pages[1].current);
    EXPECT_FALSE(model.pages[2].current);
}

TEST(PaginationModelTests, HidesSinglePageNavigation) {
    const auto model = drogular::makePaginationModel(
        1,
        1,
        [](int page) {
            return "/users?page=" + std::to_string(page);
        }
    );

    EXPECT_FALSE(model.visible);
    EXPECT_FALSE(model.hasPrevious);
    EXPECT_FALSE(model.hasNext);
    ASSERT_EQ(model.pages.size(), 1u);
}