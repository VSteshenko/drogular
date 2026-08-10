#include "todo_store.hpp"

#include <gtest/gtest.h>

#include <set>
#include <thread>
#include <vector>

TEST(TodoPweTodoStoreTests, CreatesTodo) {
    TodoStore store(std::vector<Todo>{});

    store.create("Learn State");

    ASSERT_EQ(store.snapshot().size(), 1);
    EXPECT_EQ(store.snapshot()[0].title, "Learn State");
    EXPECT_FALSE(store.snapshot()[0].done);
}

TEST(TodoPweTodoStoreTests, TogglesTodo) {
    TodoStore store(
        std::vector<Todo>{
            {1, "Test", false}
        }
    );

    store.toggle(1);

    ASSERT_EQ(store.snapshot().size(), 1);
    EXPECT_TRUE(store.snapshot()[0].done);
}

TEST(TodoPweTodoStoreTests, RemovesTodo) {
    TodoStore store(std::vector<Todo>{
        {1, "First", false},
        {2, "Second", false}
    });

    store.remove(1);

    ASSERT_EQ(store.snapshot().size(), 1);
    EXPECT_EQ(store.snapshot()[0].id, 2);
}

TEST(TodoPweTodoStoreTests, NotifiesSubscribersWhenTodoIsCreated) {
    TodoStore store(std::vector<Todo>{});

    bool notified = false;

    store.subscribe(
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

    EXPECT_TRUE(store.snapshot().empty());
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

TEST(TodoPweTodoStoreTests, SerializesConcurrentCreates) {
    TodoStore store(std::vector<Todo>{});

    constexpr int threadCount = 8;
    constexpr int todosPerThread = 100;

    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (int thread = 0; thread < threadCount; ++thread) {
        threads.emplace_back([&store, thread] {
            for (int index = 0; index < todosPerThread; ++index) {
                store.create(
                    "Todo " + std::to_string(thread) + "-" +
                    std::to_string(index)
                );
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    const auto todos = store.snapshot();
    ASSERT_EQ(todos.size(), threadCount * todosPerThread);

    std::set<int> ids;
    for (const auto& todo : todos) {
        ids.insert(todo.id);
    }

    EXPECT_EQ(ids.size(), todos.size());
}