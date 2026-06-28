#include <drogular/render_context.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

struct CoreTestTodo {
    int id;
    std::string title;
};

TEST(CoreGraphQLResultTests, StoresAndReadsValue) {
    drogular::RenderContext context;

    context.graphql().set("viewer", std::string("Vadim"));

    const auto viewer = context.graphql().get<std::string>("viewer");

    ASSERT_TRUE(viewer.has_value());
    EXPECT_EQ(*viewer, "Vadim");
}

TEST(CoreGraphQLResultTests, ReturnsNulloptForMissingValue) {
    drogular::RenderContext context;

    const auto viewer = context.graphql().get<std::string>("viewer");

    EXPECT_FALSE(viewer.has_value());
}

TEST(CoreGraphQLResultTests, ThrowsWhenRequiredValueIsMissing) {
    drogular::RenderContext context;

    EXPECT_THROW(
        context.graphql().require<std::string>("viewer"),
        drogular::RenderContextError
    );
}

TEST(CoreGraphQLResultTests, StoresVectorValue) {
    drogular::RenderContext context;

    const std::vector<CoreTestTodo> todos = {
        {1, "Create GraphQL result"},
        {2, "Use it from RenderContext"}
    };

    context.graphql().set("todos", todos);

    const auto result =
        context.graphql().require<std::vector<CoreTestTodo>>("todos");

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].id, 1);
    EXPECT_EQ(result[0].title, "Create GraphQL result");
}

TEST(CoreGraphQLResultTests, ChecksContains) {
    drogular::RenderContext context;

    context.graphql().set("viewer", std::string("Vadim"));

    EXPECT_TRUE(context.graphql().contains("viewer"));
    EXPECT_FALSE(context.graphql().contains("missing"));
}

TEST(CoreGraphQLResultTests, ClearsValues) {
    drogular::RenderContext context;

    context.graphql().set("viewer", std::string("Vadim"));
    context.graphql().clear();

    EXPECT_FALSE(context.graphql().contains("viewer"));
}

TEST(CoreGraphQLResultTests, MergesResults) {
    drogular::GraphQLResult first;

    first.set("viewer", std::string("Vadim"));
    drogular::GraphQLResult second;

    second.set("theme", std::string("dark"));
    first.merge(std::move(second));

    EXPECT_EQ(first.require<std::string>("viewer"), "Vadim");
    EXPECT_EQ(first.require<std::string>("theme"), "dark");
}

TEST(CoreGraphQLResultTests, ReplacesExistingKeys) {
    drogular::GraphQLResult first;

    first.set("viewer", std::string("Old"));
    drogular::GraphQLResult second;

    second.set("viewer", std::string("New"));
    first.merge(std::move(second));

    EXPECT_EQ(first.require<std::string>("viewer"), "New");
}