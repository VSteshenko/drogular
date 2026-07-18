#include "features/projects/ui/portal_project_query_parser.hpp"
#include "features/projects/ui/portal_project_query_serializer.hpp"

#include <drogon/HttpRequest.h>

#include <gtest/gtest.h>

TEST(PortalProjectQueryParserTests, UsesDefaultsForEmptyRequest) {
    const auto request =
        drogon::HttpRequest::newHttpRequest();

    const auto query =
        PortalProjectQueryParser::fromRequest(request);

    EXPECT_FALSE(query.search.has_value());
    EXPECT_FALSE(query.status.has_value());
    EXPECT_FALSE(query.projectTypeId.has_value());
    EXPECT_FALSE(query.ownerId.has_value());
    EXPECT_EQ(query.page, 1);
    EXPECT_EQ(query.pageSize, 10);
    ASSERT_EQ(query.sorting.size(), 1);
    EXPECT_EQ(query.sorting.front().field, "title");
    EXPECT_EQ(
        query.sorting.front().direction,
        PortalProjectSortDirection::Ascending
    );
}

TEST(PortalProjectQueryParserTests, ParsesProjectQueryParameters) {
    const auto request =
        drogon::HttpRequest::newHttpRequest();

    request->setParameter("search", "portal demo");
    request->setParameter("status", "paused");
    request->setParameter("projectTypeId", "3");
    request->setParameter("ownerId", "7");
    request->setParameter("sort", "status");
    request->setParameter("direction", "desc");
    request->setParameter("page", "4");

    const auto query =
        PortalProjectQueryParser::fromRequest(request);

    ASSERT_TRUE(query.search.has_value());
    EXPECT_EQ(*query.search, "portal demo");
    ASSERT_TRUE(query.status.has_value());
    EXPECT_EQ(*query.status, "paused");
    ASSERT_TRUE(query.projectTypeId.has_value());
    EXPECT_EQ(*query.projectTypeId, 3);
    ASSERT_TRUE(query.ownerId.has_value());
    EXPECT_EQ(*query.ownerId, 7);
    EXPECT_EQ(query.page, 4);
    ASSERT_EQ(query.sorting.size(), 1);
    EXPECT_EQ(query.sorting.front().field, "status");
    EXPECT_EQ(
        query.sorting.front().direction,
        PortalProjectSortDirection::Descending
    );
}

TEST(PortalProjectQueryParserTests, RejectsInvalidValues) {
    const auto request =
        drogon::HttpRequest::newHttpRequest();

    request->setParameter("projectTypeId", "invalid");
    request->setParameter("ownerId", "0");
    request->setParameter("sort", "unknown");
    request->setParameter("direction", "unknown");
    request->setParameter("page", "-2");

    const auto query =
        PortalProjectQueryParser::fromRequest(request);

    EXPECT_FALSE(query.projectTypeId.has_value());
    EXPECT_FALSE(query.ownerId.has_value());
    EXPECT_EQ(query.page, 1);
    ASSERT_EQ(query.sorting.size(), 1);
    EXPECT_EQ(query.sorting.front().field, "title");
    EXPECT_EQ(
        query.sorting.front().direction,
        PortalProjectSortDirection::Ascending
    );
}

TEST(PortalProjectQuerySerializerTests, OmitsDefaultValues) {
    PortalProjectQuery query;
    query.sorting.push_back({
        .field = "title",
        .direction = PortalProjectSortDirection::Ascending
    });

    EXPECT_EQ(
        PortalProjectQuerySerializer::toQueryString(query),
        ""
    );
}

TEST(PortalProjectQuerySerializerTests, SerializesAndEncodesQuery) {
    PortalProjectQuery query;
    query.search = "portal demo";
    query.status = "paused";
    query.projectTypeId = 3;
    query.ownerId = 7;
    query.sorting.push_back({
        .field = "status",
        .direction = PortalProjectSortDirection::Descending
    });
    query.page = 4;

    EXPECT_EQ(
        PortalProjectQuerySerializer::toQueryString(query),
        "?search=portal%20demo&status=paused&projectTypeId=3&ownerId=7&sort=status&direction=desc&page=4"
    );
}

TEST(PortalProjectQuerySerializerTests, SerializesPageWithoutOtherParameters) {
    PortalProjectQuery query;
    query.page = 2;

    EXPECT_EQ(
        PortalProjectQuerySerializer::toQueryString(query),
        "?page=2"
    );
}
