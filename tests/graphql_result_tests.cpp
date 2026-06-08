#include <drogular/component.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

struct TestTodo {
    int id;
    std::string title;
};

TEST(GraphQLResultTests, StoresAndReadsValue) {
    drogular::RenderContext context;

    context.graphql().set("viewer", std::string("Vadim"));

    const auto viewer = context.graphql().get<std::string>("viewer");

    ASSERT_TRUE(viewer.has_value());
    EXPECT_EQ(*viewer, "Vadim");
}

TEST(GraphQLResultTests, ReturnsNulloptForMissingValue) {
    drogular::RenderContext context;

    const auto viewer = context.graphql().get<std::string>("viewer");

    EXPECT_FALSE(viewer.has_value());
}

TEST(GraphQLResultTests, ThrowsWhenRequiredValueIsMissing) {
    drogular::RenderContext context;

    EXPECT_THROW(
        context.graphql().require<std::string>("viewer"),
        drogular::RenderContextError
    );
}

TEST(GraphQLResultTests, StoresVectorValue) {
    drogular::RenderContext context;

    const std::vector<TestTodo> todos = {
        {1, "Create GraphQL result"},
        {2, "Use it from RenderContext"}
    };

    context.graphql().set("todos", todos);

    const auto result = context.graphql().require<std::vector<TestTodo>>("todos");

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].id, 1);
    EXPECT_EQ(result[0].title, "Create GraphQL result");
}

TEST(GraphQLResultTests, ChecksContains) {
    drogular::RenderContext context;

    context.graphql().set("viewer", std::string("Vadim"));

    EXPECT_TRUE(context.graphql().contains("viewer"));
    EXPECT_FALSE(context.graphql().contains("missing"));
}

TEST(GraphQLResultTests, ClearsValues) {
    drogular::RenderContext context;

    context.graphql().set("viewer", std::string("Vadim"));
    context.graphql().clear();

    EXPECT_FALSE(context.graphql().contains("viewer"));
}

TEST(GraphQLResultTests, MergesResults) {
    drogular::GraphQLResult first;

    first.set("viewer", std::string("Vadim"));
    drogular::GraphQLResult second;

    second.set("theme", std::string("dark"));
    first.merge(std::move(second));

    EXPECT_EQ(first.require<std::string>("viewer"), "Vadim");
    EXPECT_EQ(first.require<std::string>("theme"), "dark");
}

TEST(GraphQLResultTests, ReplacesExistingKeys) {
    drogular::GraphQLResult first;

    first.set("viewer", std::string("Old"));
    drogular::GraphQLResult second;

    second.set("viewer", std::string("New"));
    first.merge(std::move(second));

    EXPECT_EQ(first.require<std::string>("viewer"), "New");
}