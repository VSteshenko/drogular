#include <drogular/graphql.hpp>

#include <gtest/gtest.h>

TEST(GraphQLMutationTests, BuildsSimpleMutation) {
    const auto mutation =
        drogular::gql::mutation("CreateTodo")
            .variable("title", "String!")
            .select(
                drogular::gql::field("createTodo")
                    .arg(
                        "title",
                        drogular::gql::variable("title")
                    )
                    .children({
                        drogular::gql::field("id"),
                        drogular::gql::field("title")
                    })
            );

    EXPECT_EQ(
        mutation.toString(),
        "mutation CreateTodo($title: String!) {\n"
        "  createTodo(title: $title) {\n"
        "    id\n"
        "    title\n"
        "  }\n"
        "}"
    );
}

TEST(GraphQLMutationTests, ValidatesEmptyMutation) {
    const auto mutation =
        drogular::gql::mutation("EmptyMutation");

    const auto validation =
        mutation.validate();

    EXPECT_FALSE(validation.valid());
}