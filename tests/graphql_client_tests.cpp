#include <drogular/graphql_client.hpp>

#include <gtest/gtest.h>

#include <string>

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
