#include "todo.hpp"
#include "todo_page.hpp"
#include "todo_item_component.hpp"
#include "todo_store.hpp"

#include <drogular/graphql_client.hpp>
#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <vector>

namespace {

void configureTodoPwaTestServices(
    drogular::ApplicationServices& services,
    drogular::ApplicationOptions& options
) {
    options.setTemplateRoot(
        "../../examples/todo_pwa/templates"
    );

    services.setOptions(&options);
}

} // namespace

TEST(TodoPwaTodoPageTests, RendersTodoList) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    configureTodoPwaTestServices(
        services,
        options
    );

    services.registerService<TodoStore>(
        std::make_shared<TodoStore>()
    );

    services.components()
        .registerComponent<TodoItemComponent>();

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

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "/todos/delete"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            renderResult.html,
            "Delete"
        )
    );
}

TEST(TodoPwaTodoPageTests, RendersEmptyTodoState) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    configureTodoPwaTestServices(
        services,
        options
    );

    services.registerService<TodoStore>(
        std::make_shared<TodoStore>(
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

TEST(TodoPweTodoServiceTests, TogglesTodo) {
    TodoStore store(std::vector<Todo>{
        {1, "Test", false}
    });

    store.toggle(1);

    ASSERT_EQ(store.todos.value().size(), 1);
    EXPECT_TRUE(store.todos.value()[0].done);

    store.toggle(1);

    EXPECT_FALSE(store.todos.value()[0].done);
}