#include <drogular/graphql.hpp>

#include <gtest/gtest.h>

TEST(GraphQLTests, BuildsEmptyQuery) {
    const auto query = drogular::gql::query("Dashboard").toString();

    EXPECT_EQ(
        query,
        "query Dashboard {\n}"
    );
}

TEST(GraphQLTests, BuildsQueryWithSingleSelection) {
    const auto query = drogular::gql::query("Dashboard")
        .select("user", {"id", "name"})
        .toString();

    EXPECT_EQ(
        query,
R"(query Dashboard {
  user {
    id
    name
  }
})"
    );
}

TEST(GraphQLTests, BuildsQueryWithMultipleSelections) {
    const auto query = drogular::gql::query("Dashboard")
        .select("user", {"id", "name", "avatarUrl"})
        .select("todos", {"id", "title", "done"})
        .toString();

    EXPECT_EQ(
        query,
R"(query Dashboard {
  user {
    id
    name
    avatarUrl
  }
  todos {
    id
    title
    done
  }
})"
    );
}
