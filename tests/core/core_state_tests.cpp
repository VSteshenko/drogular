#include <drogular/state.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(CoreStateTests, StoresInitialValue) {
    drogular::State<int> state(42);

    EXPECT_EQ(state.value(), 42);
}

TEST(CoreStateTests, UpdatesValue) {
    drogular::State<int> state(1);

    state.set(2);

    EXPECT_EQ(state.value(), 2);
}

TEST(CoreStateTests, StoresStringValue) {
    drogular::State<std::string> state("hello");

    EXPECT_EQ(state.value(), "hello");
}

TEST(CoreStateTests, StoresVectorValue) {
    drogular::State<std::vector<int>> state({
        1,
        2,
        3
    });

    EXPECT_EQ(state.value().size(), 3);
}

TEST(CoreStateTests, AllowsMutableAccess) {
    drogular::State<std::vector<int>> state;

    state.value().push_back(1);
    state.value().push_back(2);

    EXPECT_EQ(state.value().size(), 2);
}

TEST(CoreStateTests, NotifiesSubscriber) {
    drogular::State<int> state(0);

    int received = -1;

    state.subscribe(
        [&](const int& value) {
            received = value;
        }
    );

    state.set(42);

    EXPECT_EQ(received, 42);
}

TEST(CoreStateTests, NotifiesAllSubscribers) {
    drogular::State<int> state(0);

    int first = 0;
    int second = 0;

    state.subscribe(
        [&](const int& value) {
            first = value;
        }
    );

    state.subscribe(
        [&](const int& value) {
            second = value;
        }
    );

    state.set(100);

    EXPECT_EQ(first, 100);
    EXPECT_EQ(second, 100);
}

TEST(CoreStateTests, DoesNotNotifyBeforeSet) {
    drogular::State<int> state(0);

    bool called = false;

    state.subscribe(
        [&](const int&) {
            called = true;
        }
    );

    EXPECT_FALSE(called);
}

TEST(CoreStateTests, NotifiesForEveryChange) {
    drogular::State<int> state(0);

    std::vector<int> values;

    state.subscribe(
        [&](const int& value) {
            values.push_back(value);
        }
    );

    state.set(1);
    state.set(2);
    state.set(3);

    const std::vector<int> expected = {
        1,
        2,
        3
    };

    EXPECT_EQ(values, expected);
}