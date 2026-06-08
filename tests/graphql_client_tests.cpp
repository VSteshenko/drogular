#include <drogular/component.hpp>
#include <drogular/services.hpp>
#include <drogular/graphql_client.hpp>

#include <gtest/gtest.h>

#include <string>

TEST(GraphQLClientTests, RenderContextExecutesGraphQLThroughServices) {
    drogular::GraphQLResult result;
    result.set("viewer", std::string("Vadim"));

    auto client = std::make_shared<drogular::StaticGraphQLClient>(result);

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
        context.graphql().require<std::string>("viewer"),
        "Vadim"
    );
}

TEST(GraphQLClientTests, RenderContextCanStoreGraphQLClient) {
    drogular::GraphQLResult result;
    drogular::StaticGraphQLClient client(result);

    drogular::RenderContext context;

    context.setGraphQLClient(&client);

    EXPECT_TRUE(context.hasGraphQLClient());
}

TEST(GraphQLClientTests, RenderContextExecutesGraphQLQuery) {
    drogular::GraphQLResult result;
    result.set("viewer", std::string("Vadim"));

    drogular::StaticGraphQLClient client(result);

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
        context.graphql().require<std::string>("viewer"),
        "Vadim"
    );
}

TEST(GraphQLClientTests, RenderContextThrowsWithoutGraphQLClient) {
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

TEST(GraphQLClientTests, StaticClientReturnsPredefinedResult) {
    drogular::GraphQLResult result;
    result.set("viewer", std::string("Vadim"));

    drogular::StaticGraphQLClient client(result);

    const auto query = drogular::gql::query("Viewer")
        .select(
            drogular::gql::field("viewer")
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("name")
                })
        );

    const auto response = client.execute(query);

    EXPECT_EQ(
        response.require<std::string>("viewer"),
        "Vadim"
    );
}

TEST(
    GraphQLClientTests,
    ExecuteGraphQLMergesResults
) {
    drogular::GraphQLResult existing;

    existing.set("viewer", std::string("Vadim"));
    drogular::GraphQLResult incoming;

    incoming.set("theme", std::string("dark"));
    drogular::StaticGraphQLClient client(std::move(incoming));
    drogular::RenderContext context;

    context.graphql().merge(std::move(existing));
    context.setGraphQLClient(&client);

    auto query = drogular::gql::query("Settings");
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

TEST(GraphQLClientTests, CreatesHttpGraphQLClient) {
    drogular::HttpGraphQLClient client(
        "localhost",
        8080,
        "/graphql"
    );

    SUCCEED();
}