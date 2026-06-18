#include "../examples/todo_pwa/todo_store.hpp"

#include <gtest/gtest.h>

TEST(TodoPweTodoStoreTests, CreatesTodo) {
    TodoStore store(std::vector<Todo>{});

    store.create("Learn State");

    ASSERT_EQ(store.todos.value().size(), 1);
    EXPECT_EQ(store.todos.value()[0].title, "Learn State");
    EXPECT_FALSE(store.todos.value()[0].done);
}

TEST(TodoPweTodoStoreTests, TogglesTodo) {
    TodoStore store(
        std::vector<Todo>{
            {1, "Test", false}
        }
    );

    store.toggle(1);

    ASSERT_EQ(store.todos.value().size(), 1);
    EXPECT_TRUE(store.todos.value()[0].done);
}

TEST(TodoPweTodoStoreTests, RemovesTodo) {
    TodoStore store(std::vector<Todo>{
        {1, "First", false},
        {2, "Second", false}
    });

    store.remove(1);

    ASSERT_EQ(store.todos.value().size(), 1);
    EXPECT_EQ(store.todos.value()[0].id, 2);
}

TEST(TodoPweTodoStoreTests, NotifiesSubscribersWhenTodoIsCreated) {
    TodoStore store(std::vector<Todo>{});

    bool notified = false;

    store.todos.subscribe(
        [&](const std::vector<Todo>& todos) {
            notified = true;
            EXPECT_EQ(todos.size(), 1);
        }
    );

    store.create("Learn State");

    EXPECT_TRUE(notified);
}

TEST(TodoPweTodoStoreTests, DoesNotCreateEmptyTodo) {
    TodoStore store(std::vector<Todo>{});

    store.create("");

    EXPECT_TRUE(store.todos.value().empty());
}