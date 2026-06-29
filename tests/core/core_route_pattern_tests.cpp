#include <drogular/route_pattern.hpp>

#include <gtest/gtest.h>

#include <unordered_map>

TEST(CoreRoutePatternTests, MatchesStaticPath) {
    drogular::RoutePattern pattern(
        "/projects"
    );

    std::unordered_map<std::string, std::string> params;

    EXPECT_TRUE(
        pattern.match(
            "/projects",
            params
        )
    );

    EXPECT_TRUE(params.empty());
}

TEST(CoreRoutePatternTests, MatchesSingleParameter) {
    drogular::RoutePattern pattern(
        "/projects/{id}"
    );

    std::unordered_map<std::string, std::string> params;

    EXPECT_TRUE(
        pattern.match(
            "/projects/42",
            params
        )
    );

    EXPECT_EQ(
        params["id"],
        "42"
    );
}

TEST(CoreRoutePatternTests, MatchesMultipleParameters) {
    drogular::RoutePattern pattern(
        "/projects/{projectId}/tasks/{taskId}"
    );

    std::unordered_map<std::string, std::string> params;

    EXPECT_TRUE(
        pattern.match(
            "/projects/10/tasks/25",
            params
        )
    );

    EXPECT_EQ(params["projectId"], "10");
    EXPECT_EQ(params["taskId"], "25");
}

TEST(CoreRoutePatternTests, RejectsMissingSegment) {
    drogular::RoutePattern pattern(
        "/projects/{id}"
    );

    std::unordered_map<std::string, std::string> params;

    EXPECT_FALSE(
        pattern.match(
            "/projects",
            params
        )
    );
}

TEST(CoreRoutePatternTests, RejectsExtraSegment) {
    drogular::RoutePattern pattern(
        "/projects/{id}"
    );

    std::unordered_map<std::string, std::string> params;

    EXPECT_FALSE(
        pattern.match(
            "/projects/42/edit",
            params
        )
    );
}

TEST(CoreRoutePatternTests, RejectsDifferentStaticSegment) {
    drogular::RoutePattern pattern(
        "/projects/{id}"
    );

    std::unordered_map<std::string, std::string> params;

    EXPECT_FALSE(
        pattern.match(
            "/users/42",
            params
        )
    );
}