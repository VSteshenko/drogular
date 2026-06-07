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
        .select("user", {
            drogular::gql::field("id"),
            drogular::gql::field("name")
        })
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
        .select("user", {
            drogular::gql::field("id"),
            drogular::gql::field("name"),
            drogular::gql::field("avatarUrl")
        })
        .select("todos", {
            drogular::gql::field("id"),
            drogular::gql::field("title"),
            drogular::gql::field("done")
        })
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

TEST(GraphQLTests, BuildsNestedSelections) {
    const auto query = drogular::gql::query("Dashboard")
        .select("user", {
            drogular::gql::field("id"),
            drogular::gql::field("name"),
            drogular::gql::field("company", {
                drogular::gql::field("id"),
                drogular::gql::field("title")
            })
        })
        .toString();

    EXPECT_EQ(
        query,
R"(query Dashboard {
  user {
    id
    name
    company {
      id
      title
    }
  }
})"
    );
}

TEST(GraphQLTests, BuildsFieldWithArgument) {
    const auto query = drogular::gql::query("Dashboard")
        .select(
            drogular::gql::field("user")
                .arg("id", drogular::gql::string("42"))
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("name")
                })
        )
        .toString();

    EXPECT_EQ(
        query,
R"(query Dashboard {
  user(id: "42") {
    id
    name
  }
})"
    );
}

TEST(GraphQLTests, BuildsFieldWithMultipleArguments) {
    const auto query = drogular::gql::query("Dashboard")
        .select(
            drogular::gql::field("todos")
                .arg("limit", drogular::gql::intValue(10))
                .arg("done", drogular::gql::boolValue(false))
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("title")
                })
        )
        .toString();

    EXPECT_EQ(
        query,
R"(query Dashboard {
  todos(limit: 10, done: false) {
    id
    title
  }
})"
    );
}

TEST(GraphQLTests, EscapesStringValues) {
    const auto query = drogular::gql::query("Search")
        .select(
            drogular::gql::field("search")
                .arg("text", drogular::gql::string(R"(hello "world")"))
                .children({
                    drogular::gql::field("id")
                })
        )
        .toString();

    EXPECT_EQ(
        query,
R"(query Search {
  search(text: "hello \"world\"") {
    id
  }
})"
    );
}
