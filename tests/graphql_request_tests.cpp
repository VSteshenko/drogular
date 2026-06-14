#include <drogular/graphql_request.hpp>

#include <gtest/gtest.h>

TEST(GraphQLRequestTests, StoresQuery) {
    drogular::GraphQLRequest request("query { viewer { id } }");

    EXPECT_EQ(
        request.query(),
        "query { viewer { id } }"
    );
}

TEST(GraphQLRequestTests, UpdatesQuery) {
    drogular::GraphQLRequest request;

    request.setQuery("query { todos { id } }");

    EXPECT_EQ(
        request.query(),
        "query { todos { id } }"
    );
}

TEST(GraphQLRequestTests, StoresVariables) {
    drogular::GraphQLRequest request;

    request.setVariable("userId", "123");
    request.setVariable("limit", 10);
    request.setVariable("done", false);

    EXPECT_EQ(
        request.variables()["userId"].asString(),
        "123"
    );

    EXPECT_EQ(
        request.variables()["limit"].asInt(),
        10
    );

    EXPECT_FALSE(
        request.variables()["done"].asBool()
    );
}

TEST(GraphQLRequestTests, ConvertsToJson) {
    drogular::GraphQLRequest request("query User($id: ID!) { user(id: $id) { name } }");

    request.setVariable("id", "42");

    const auto json = request.toJson();

    EXPECT_EQ(
        json["query"].asString(),
        "query User($id: ID!) { user(id: $id) { name } }"
    );

    EXPECT_EQ(
        json["variables"]["id"].asString(),
        "42"
    );
}

TEST(GraphQLRequestTests, SupportsFluentApi) {
    drogular::GraphQLRequest request;

    request
        .query("query User($id: ID!) { user(id: $id) { name } }")
        .variable("id", "42")
        .variable("limit", 10);

    EXPECT_EQ(
        request.query(),
        "query User($id: ID!) { user(id: $id) { name } }"
    );

    EXPECT_EQ(
        request.variables()["id"].asString(),
        "42"
    );

    EXPECT_EQ(
        request.variables()["limit"].asInt(),
        10
    );
}