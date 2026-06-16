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

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "/todos/toggle"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "name=\"id\""
        )
    );
}

TEST(TodoPageTests, RendersEmptyTodoState) {
    drogular::ApplicationServices services;
    services.registerService<TodoService>(
        std::make_shared<TodoService>(
            std::vector<Todo>{}
        )
    );
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

TEST(TodoServiceTests, TogglesTodo) {
    TodoService service({
        {1, "Test", false}
    });

    service.toggle(1);

    ASSERT_EQ(service.todos().size(), 1);
    EXPECT_TRUE(service.todos()[0].done);

    service.toggle(1);

    EXPECT_FALSE(service.todos()[0].done);
}