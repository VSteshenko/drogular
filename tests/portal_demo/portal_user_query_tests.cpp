#include "features/users/providers/portal_memory_user_provider.hpp"
#include "features/users/ui/portal_user_query_parser.hpp"
#include "features/users/ui/portal_user_query_serializer.hpp"

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

TEST(PortalUserQueryParserTests, UsesDefaultsForEmptyRequest) {
    const auto request =
        drogon::HttpRequest::newHttpRequest();
    const auto query =
        PortalUserQueryParser::fromRequest(request);

    EXPECT_FALSE(query.search.has_value());
    EXPECT_FALSE(query.role.has_value());
    EXPECT_EQ(query.page, 1);
    EXPECT_EQ(query.pageSize, 10);
    ASSERT_EQ(query.sorting.size(), 1);
    EXPECT_EQ(query.sorting.front().field, "username");
    EXPECT_EQ(
        query.sorting.front().direction,
        PortalUserSortDirection::Ascending
    );
}

TEST(PortalUserQueryParserTests, ParsesParameters) {
    const auto request =
        drogon::HttpRequest::newHttpRequest();

    request->setParameter("search", "adm");
    request->setParameter("role", "admin");
    request->setParameter("sort", "role");
    request->setParameter("direction", "desc");
    request->setParameter("page", "2");
    request->setParameter("pageSize", "5");

    const auto query =
        PortalUserQueryParser::fromRequest(request);

    ASSERT_TRUE(query.search.has_value());
    EXPECT_EQ(*query.search, "adm");
    ASSERT_TRUE(query.role.has_value());
    EXPECT_EQ(*query.role, "admin");
    EXPECT_EQ(query.page, 2);
    EXPECT_EQ(query.pageSize, 5);
    EXPECT_EQ(query.sorting.front().field, "role");
    EXPECT_EQ(
        query.sorting.front().direction,
        PortalUserSortDirection::Descending
    );
}

TEST(PortalUserQueryParserTests, RejectsInvalidValues) {
    const auto request =
        drogon::HttpRequest::newHttpRequest();

    request->setParameter("sort", "unknown");
    request->setParameter("direction", "unknown");
    request->setParameter("page", "0");
    request->setParameter("pageSize", "invalid");

    const auto query = PortalUserQueryParser::fromRequest(request);

    EXPECT_EQ(query.page, 1);
    EXPECT_EQ(query.pageSize, 10);
    EXPECT_EQ(query.sorting.front().field, "username");
    EXPECT_EQ(
        query.sorting.front().direction,
        PortalUserSortDirection::Ascending
    );
}

TEST(PortalUserQuerySerializerTests, OmitsDefaults) {
    PortalUserQuery query;
    query.sorting.push_back({
        .field = "username",
        .direction = PortalUserSortDirection::Ascending
    });

    EXPECT_EQ(PortalUserQuerySerializer::toQueryString(query), "");
}

TEST(PortalUserQuerySerializerTests, SerializesAndEncodesQuery) {
    PortalUserQuery query;
    query.search = "portal admin";
    query.role = "admin";
    query.sorting.push_back({
        .field = "role",
        .direction = PortalUserSortDirection::Descending
    });
    query.page = 3;
    query.pageSize = 5;

    EXPECT_EQ(
        PortalUserQuerySerializer::toQueryString(query),
        "?search=portal%20admin&role=admin&sort=role&direction=desc&pageSize=5&page=3"
    );
}

TEST(PortalMemoryUserProviderTests, SearchesSortsAndPaginates) {
    PortalMemoryUserProvider provider({
        {3, "zoe", "secret", "user"},
        {1, "admin", "secret", "admin"},
        {2, "alice", "secret", "user"},
        {4, "adam", "secret", "admin"}
    });

    PortalUserQuery query;
    query.search = "a";
    query.sorting.push_back({
        .field = "username",
        .direction = PortalUserSortDirection::Ascending
    });
    query.pageSize = 2;

    const auto first = provider.search(query);
    ASSERT_EQ(first.items.size(), 2);
    EXPECT_EQ(first.items[0].username, "adam");
    EXPECT_EQ(first.items[1].username, "admin");
    EXPECT_EQ(first.totalItems, 3);
    EXPECT_EQ(first.totalPages, 2);

    query.page = 2;
    const auto second = provider.search(query);
    ASSERT_EQ(second.items.size(), 1);
    EXPECT_EQ(second.items[0].username, "alice");
}

TEST(PortalMemoryUserProviderTests, FiltersByRole) {
    PortalMemoryUserProvider provider({
        {1, "admin", "secret", "admin"},
        {2, "alice", "secret", "user"}
    });

    PortalUserQuery query;
    query.role = "admin";

    const auto result = provider.search(query);
    ASSERT_EQ(result.items.size(), 1);
    EXPECT_EQ(result.items.front().username, "admin");
}