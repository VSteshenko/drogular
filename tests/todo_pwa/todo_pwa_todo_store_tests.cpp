#include "todo_store.hpp"

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
TEST(TodoPweTodoStoreTests, FiltersTodosCaseInsensitively) {
    TodoStore store(std::vector<Todo>{
        {1, "Add Search", false},
        {2, "Write tests", false},
        {3, "Improve SEARCH experience", false}
    });

    const auto result = store.find(TodoQuery{
        .search = "search",
        .page = 1,
        .pageSize = 10
    });

    ASSERT_EQ(result.items.size(), 2);
    EXPECT_EQ(result.totalItems, 2);
}

TEST(TodoPweTodoStoreTests, PaginatesAndClampsRequestedPage) {
    TodoStore store(std::vector<Todo>{
        {1, "One", false},
        {2, "Two", false},
        {3, "Three", false},
        {4, "Four", false},
        {5, "Five", false},
        {6, "Six", false}
    });

    const auto result = store.find(TodoQuery{
        .page = 99,
        .pageSize = 5
    });

    EXPECT_EQ(result.page, 2);
    EXPECT_EQ(result.totalPages, 2);
    ASSERT_EQ(result.items.size(), 1);
    EXPECT_EQ(result.items.front().title, "Six");
}