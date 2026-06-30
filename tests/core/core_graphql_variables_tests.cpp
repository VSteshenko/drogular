#include <drogular/graphql_variables.hpp>

#include <gtest/gtest.h>

TEST(CoreGraphQLVariablesTests, StoresStringValue) {
    drogular::GraphQLVariables variables;

    variables.set("title", "Portal");

    EXPECT_EQ(
        variables.json()["title"].asString(),
        "Portal"
    );
}

TEST(CoreGraphQLVariablesTests, StoresObjectValue) {
    Json::Value project;
    project["id"] = 1;
    project["title"] = "Portal";
    project["status"] = "active";

    drogular::GraphQLVariables variables;

    variables.set("project", project);

    EXPECT_EQ(
        variables.json()["project"]["title"].asString(),
        "Portal"
    );
}