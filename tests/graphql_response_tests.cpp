#include <drogular/graphql_response.hpp>

#include <gtest/gtest.h>

TEST(GraphQLResponseTests, ReturnsData) {
    Json::Value json;

    json["data"]["user"]["name"] = "Vadim";

    drogular::GraphQLResponse response(json);

    EXPECT_EQ(
        response.data()["user"]["name"].asString(),
        "Vadim"
    );
}

TEST(GraphQLResponseTests, ReturnsErrors) {
    Json::Value json;

    json["errors"] = Json::arrayValue;
    json["errors"].append("Something failed");

    drogular::GraphQLResponse response(json);

    EXPECT_EQ(
        response.errors().size(),
        1
    );
}

TEST(GraphQLResponseTests, DetectsErrors) {
    Json::Value json;

    json["errors"] = Json::arrayValue;
    json["errors"].append("Error");

    drogular::GraphQLResponse response(json);

    EXPECT_TRUE(response.hasErrors());
}

TEST(GraphQLResponseTests, DetectsNoErrors) {
    Json::Value json;

    json["data"]["ok"] = true;

    drogular::GraphQLResponse response(json);

    EXPECT_FALSE(response.hasErrors());
}

TEST(GraphQLResponseTests, ReturnsRawJson) {
    Json::Value json;

    json["data"]["version"] = "1.0";

    drogular::GraphQLResponse response(json);

    EXPECT_EQ(
        response.rawJson()["data"]["version"].asString(),
        "1.0"
    );
}