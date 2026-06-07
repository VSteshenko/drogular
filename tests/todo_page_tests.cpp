#include "todo_page.hpp"

#include <drogular/testing.hpp>

#include <gtest/gtest.h>

TEST(TodoPageTests, RendersTodoList) {
    const auto result = drogular::test::renderPage<TodoPage>();

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Drogular Todo PWA"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Todo list"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Create Drogular project skeleton"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Test TodoPage rendering"
        )
    );
}