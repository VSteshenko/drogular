#include "todo.hpp"
#include "todo_page.hpp"
#include "todo_item_component.hpp"
#include "todo_service.hpp"

#include <drogular/graphql_client.hpp>
#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <vector>

TEST(TodoPageTests, RendersTodoList) {
    const std::vector<Todo> todos = {
        {1, "Create Drogular project skeleton", true},
        {2, "Test TodoPage rendering", false}
    };

    drogular::GraphQLResult result;
    result.set("todos", todos);

    auto client = std::make_shared<drogular::StaticGraphQLClient>(
        std::move(result)
    );

    drogular::ApplicationServices services;
    services.add<TodoService>(
        drogular::ServiceLifetime::Singleton
    );
    services.components().registerComponent<TodoItemComponent>();

    const auto renderResult =
        drogular::test::renderPage<TodoPage>(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "Create Drogular project skeleton"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "Build TodoPWA example"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "Add actions and mutations"
        )
    );
}

TEST(TodoPageTests, RendersEmptyTodoState) {
    const std::vector<Todo> todos = {};

    drogular::GraphQLResult result;
    result.set("todos", todos);

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            std::move(result)
        );

    drogular::ApplicationServices services;
    services.registerService<TodoService>(
        std::make_shared<TodoService>(
            std::vector<Todo>{}
        )
    );
    services.setGraphQLClient(client);
    services.components().registerComponent<TodoItemComponent>();

    const auto renderResult =
        drogular::test::renderPage<TodoPage>(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "No todos yet."
        )
    );
}