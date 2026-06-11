#include "todo.hpp"
#include "todo_page.hpp"
#include "todo_item_component.hpp"

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
    services.setGraphQLClient(client);
    services.components().registerComponent<TodoItemComponent>();

    const auto renderResult =
        drogular::test::renderPage<TodoPage>(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "Drogular Todo PWA"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "Todo list"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "Create Drogular project skeleton"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "Test TodoPage rendering"
        )
    );
}