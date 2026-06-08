#include <drogular/graphql_client.hpp>
#include <drogular/services.hpp>

#include <gtest/gtest.h>

TEST(ServicesTests, StoresGraphQLClient) {
    drogular::ApplicationServices services;
    drogular::GraphQLResult result;

    auto client = std::make_shared<drogular::StaticGraphQLClient>(result);

    services.setGraphQLClient(client);

    EXPECT_NE(services.graphQLClient(), nullptr);
}
