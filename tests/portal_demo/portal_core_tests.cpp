#include "core/portal_sort_direction.hpp"
#include "core/portal_string_utils.hpp"

#include <drogular/pagination.hpp>
#include <drogular/query_string_builder.hpp>

#include <gtest/gtest.h>

TEST(PortalCoreTests, BuildsEncodedQueryString) {
    drogular::QueryStringBuilder builder;
    builder
        .add("search", "Research & Development")
        .add("page", 2);

    EXPECT_EQ(
        builder.build(),
        "?search=Research%20%26%20Development&page=2"
    );
}

TEST(PortalCoreTests, PaginatesValues) {
    const std::vector<int> values{1, 2, 3, 4, 5};
    const auto page =
        drogular::paginate(values, 2, 2);

    EXPECT_EQ(page.page, 2);
    EXPECT_EQ(page.pageSize, 2);
    EXPECT_EQ(page.totalItems, 5);
    EXPECT_EQ(page.totalPages, 3);
    EXPECT_EQ(page.items, (std::vector<int>{3, 4}));
}

TEST(PortalCoreTests, KeepsAtLeastOneTotalPage) {
    const auto page = drogular::paginate(
        std::vector<int>{},
        1,
        10);

    EXPECT_EQ(page.totalPages, 1);
    EXPECT_TRUE(page.items.empty());
}

TEST(PortalCoreTests, NormalizesAsciiCase) {
    EXPECT_EQ(
        portalAsciiLowercase("Portal DEMO"),
        "portal demo"
    );
}

TEST(PortalCoreTests, SerializesSortDirection) {
    EXPECT_STREQ(
        toString(PortalSortDirection::Ascending),
        "asc"
    );
    EXPECT_STREQ(
        toString(PortalSortDirection::Descending),
        "desc"
    );
}