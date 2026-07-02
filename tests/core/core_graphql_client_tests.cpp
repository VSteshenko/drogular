#include <drogular/component.hpp>
#include <drogular/services.hpp>
#include <drogular/graphql_client.hpp>
#include <drogular/graphql.hpp>

#include <gtest/gtest.h>

#include <string>
#include <json/json.h>
#include <json/value.h>

TEST(CoreGraphQLClientTests, RenderContextExecutesGraphQLThroughServices) {
    Json::Value viewer(Json::objectValue);
    viewer["id"] = 1;
    viewer["name"] = "Vadim";

    Json::Value data(Json::objectValue);
    data["viewer"] = viewer;
    auto client = std::make_shared<drogular::StaticGraphQLClient>(data);

    drogular::ApplicationServices services;
    services.setGraphQLClient(client);

    drogular::RenderContext context;
    context.setServices(&services);

    const auto query = drogular::gql::query("Viewer")
        .select(
            drogular::gql::field("viewer")
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("name")
                })
        );

    context.executeGraphQL(query);

    EXPECT_EQ(
        context.graphql()
            .require<Json::Value>("viewer")["name"]
            .asString(),
        "Vadim"
    );
}

TEST(CoreGraphQLClientTests, RenderContextCanStoreGraphQLClient) {
    Json::Value data(Json::objectValue);
    drogular::StaticGraphQLClient client(data);

    drogular::RenderContext context;

    context.setGraphQLClient(&client);

    EXPECT_TRUE(context.hasGraphQLClient());
}

TEST(CoreGraphQLClientTests, RenderContextExecutesGraphQLQuery) {
    Json::Value viewer(Json::objectValue);
    viewer["id"] = 1;
    viewer["name"] = "Vadim";

    Json::Value data(Json::objectValue);
    data["viewer"] = viewer;

    drogular::StaticGraphQLClient client(data);

    drogular::RenderContext context;
    context.setGraphQLClient(&client);

    const auto query = drogular::gql::query("Viewer")
        .select(
            drogular::gql::field("viewer")
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("name")
                })
        );

    context.executeGraphQL(query);

    EXPECT_EQ(
        context.graphql()
            .require<Json::Value>("viewer")["name"]
            .asString(),
        "Vadim"
    );
}

TEST(CoreGraphQLClientTests, RenderContextThrowsWithoutGraphQLClient) {
    drogular::RenderContext context;

    const auto query = drogular::gql::query("Viewer")
        .select(
            drogular::gql::field("viewer")
                .children({
                    drogular::gql::field("id")
                })
        );

    EXPECT_THROW(
        context.executeGraphQL(query),
        drogular::RenderContextError
    );
}

TEST(CoreGraphQLClientTests, StaticClientReturnsPredefinedResult) {
    Json::Value viewer(Json::objectValue);
    viewer["id"] = 1;
    viewer["name"] = "Vadim";

    Json::Value data(Json::objectValue);
    data["viewer"] = viewer;

    drogular::StaticGraphQLClient client(data);

    const auto query = drogular::gql::query("Viewer")
        .select(
            drogular::gql::field("viewer")
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("name")
                })
        );

    const auto response = client.execute(query);

    ASSERT_TRUE(response.field("viewer").has_value());

    EXPECT_EQ(
        response.field("viewer")->operator[]("name").asString(),
        "Vadim"
    );
}

TEST(CoreGraphQLClientTests, ExecuteGraphQLMergesResults) {
    drogular::GraphQLResult existing;

    existing.set("viewer", std::string("Vadim"));

    Json::Value incoming(Json::objectValue);
    incoming["theme"] = "dark";

    drogular::StaticGraphQLClient client(
        incoming
    );

    drogular::RenderContext context;

    context.graphql().merge(std::move(existing));
    context.setGraphQLClient(&client);

    auto query =
        drogular::gql::query("Settings");

    context.executeGraphQL(query);

    EXPECT_EQ(
        context.graphql()
            .require<std::string>("viewer"),
        "Vadim"
    );

    EXPECT_EQ(
        context.graphql()
            .require<std::string>("theme"),
        "dark"
    );
}

TEST(CoreGraphQLClientTests, CreatesHttpGraphQLClient) {
    drogular::HttpGraphQLClient client(
        "localhost",
        8080,
        "/graphql"
    );

    SUCCEED();
}

TEST(CoreGraphQLClientTests, StaticClientExecutesRequest) {
    Json::Value data(Json::objectValue);
    data["viewer"] = "Vadim";

    drogular::StaticGraphQLClient client(data);

    drogular::GraphQLRequest request("query { viewer { name } }");

    const auto response = client.executeRequest(request);

    EXPECT_FALSE(response.hasErrors());
}

TEST(CoreGraphQLClientTests, CreatesHttpGraphQLRequest) {
    drogular::HttpGraphQLClient client(
        "localhost",
        8080,
        "/graphql"
    );

    drogular::GraphQLRequest request("query { viewer { name } }");

    SUCCEED();
}

TEST(CoreGraphQLClientTests, GraphQLClientErrorStoresMessage) {
    drogular::GraphQLClientError error("Client failed");

    EXPECT_STREQ(
        error.what(),
        "Client failed"
    );
}

TEST(CoreGraphQLClientTests, GraphQLResponseProvidesErrorMessages) {
    Json::Value json;

    Json::Value error;
    error["message"] = "GraphQL validation failed";

    json["errors"] = Json::arrayValue;
    json["errors"].append(error);

    drogular::GraphQLResponse response(json);

    ASSERT_TRUE(response.hasErrors());

    const auto messages = response.errorMessages();

    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0], "GraphQL validation failed");
}

TEST(CoreGraphQLClientTests, ExecutesQueryWithVariables) {
    Json::Value data(Json::objectValue);
    data["projectId"] = 1;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    drogular::GraphQLVariables variables;
    variables.set("id", 1);

    const auto response =
        client->execute(
            drogular::gql::query("ProjectById")
                .variable("id", "ID!")
                .select(drogular::gql::field("projectId")),
            variables
        );

    ASSERT_TRUE(response.field("projectId").has_value());
    EXPECT_EQ(response.field("projectId")->asInt(), 1);
}

TEST(CoreGraphQLClientTests, ExecutesMutationWithVariables) {
    Json::Value data(Json::objectValue);
    data["updated"] = true;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    drogular::GraphQLVariables variables;

    Json::Value project(Json::objectValue);
    variables.set("project", project);

    const auto response =
        client->execute(
            drogular::gql::mutation("UpdateProject")
                .variable("project", "ProjectInput!")
                .select(drogular::gql::field("updated")),
            variables
        );

    ASSERT_TRUE(response.field("updated").has_value());
    EXPECT_TRUE(response.field("updated")->asBool());
}