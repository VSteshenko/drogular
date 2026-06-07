#include <drogular/graphql.hpp>

#include <gtest/gtest.h>

TEST(GraphQLTests, QueryHasCorrectName) {
    drogular::gql::Query query("Dashboard");

    EXPECT_EQ(
        query.toString(),
        "query Dashboard {\n}"
    );
}
