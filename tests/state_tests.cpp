#include <drogular/state.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(StateTests, StoresInitialValue) {
    drogular::State<int> state(42);

    EXPECT_EQ(state.value(), 42);
}

TEST(StateTests, UpdatesValue) {
    drogular::State<int> state(1);

    state.set(2);

    EXPECT_EQ(state.value(), 2);
}

TEST(StateTests, StoresStringValue) {
    drogular::State<std::string> state("hello");

    EXPECT_EQ(state.value(), "hello");
}

TEST(StateTests, StoresVectorValue) {
    drogular::State<std::vector<int>> state({
        1,
        2,
        3
    });

    EXPECT_EQ(state.value().size(), 3);
}

TEST(StateTests, AllowsMutableAccess) {
    drogular::State<std::vector<int>> state;

    state.value().push_back(1);
    state.value().push_back(2);

    EXPECT_EQ(state.value().size(), 2);
}