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

TEST(GraphQLResponseTests, DetectsData) {
    Json::Value json;

    json["data"]["viewer"]["name"] = "Vadim";

    drogular::GraphQLResponse response(json);

    EXPECT_TRUE(response.hasData());
}

TEST(GraphQLResponseTests, DetectsMissingData) {
    Json::Value json;

    drogular::GraphQLResponse response(json);

    EXPECT_FALSE(response.hasData());
}

TEST(GraphQLResponseTests, ReturnsDataField) {
    Json::Value json;

    json["data"]["viewer"]["name"] = "Vadim";

    drogular::GraphQLResponse response(json);

    const auto field = response.field("viewer");

    ASSERT_TRUE(field.has_value());
    EXPECT_EQ((*field)["name"].asString(), "Vadim");
}

TEST(GraphQLResponseTests, ReturnsNulloptForMissingDataField) {
    Json::Value json;

    json["data"]["viewer"]["name"] = "Vadim";

    drogular::GraphQLResponse response(json);

    const auto field = response.field("missing");

    EXPECT_FALSE(field.has_value());
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

TEST(GraphQLResponseTests, ReturnsStringErrorMessages) {
    Json::Value json;

    json["errors"] = Json::arrayValue;
    json["errors"].append("Plain error");

    drogular::GraphQLResponse response(json);

    const auto messages = response.errorMessages();

    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0], "Plain error");
}

TEST(GraphQLResponseTests, ReturnsObjectErrorMessages) {
    Json::Value json;

    Json::Value error;
    error["message"] = "GraphQL error";

    json["errors"] = Json::arrayValue;
    json["errors"].append(error);

    drogular::GraphQLResponse response(json);

    const auto messages = response.errorMessages();

    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0], "GraphQL error");
}

TEST(GraphQLResponseTests, ConvertsDataToGraphQLResult) {
    Json::Value json;

    json["data"]["viewer"]["name"] = "Vadim";

    drogular::GraphQLResponse response(json);

    const auto result = response.toResult();

    const auto viewer =
        result.require<Json::Value>("viewer");

    EXPECT_EQ(
        viewer["name"].asString(),
        "Vadim"
    );
}

TEST(GraphQLResponseTests, StoresRawJsonInGraphQLResult) {
    Json::Value json;

    json["data"]["viewer"]["name"] = "Vadim";

    drogular::GraphQLResponse response(json);

    const auto result = response.toResult();

    const auto rawJson =
        result.require<Json::Value>("__json");

    EXPECT_EQ(
        rawJson["data"]["viewer"]["name"].asString(),
        "Vadim"
    );
}

TEST(GraphQLResponseTests, DetectsExtensions) {
    Json::Value json;

    json["extensions"]["traceId"] = "abc-123";

    drogular::GraphQLResponse response(json);

    EXPECT_TRUE(response.hasExtensions());
}

TEST(GraphQLResponseTests, ReturnsExtensions) {
    Json::Value json;

    json["extensions"]["traceId"] = "abc-123";

    drogular::GraphQLResponse response(json);

    EXPECT_EQ(
        response.extensions()["traceId"].asString(),
        "abc-123"
    );
}