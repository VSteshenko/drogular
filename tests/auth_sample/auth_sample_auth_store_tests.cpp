#include "../examples/auth_sample/auth_store.hpp"

#include <gtest/gtest.h>

TEST(AuthSampleAuthStoreTests, IsLoggedOutByDefault) {
    AuthStore store;

    EXPECT_FALSE(
        store.currentUser.value()
            .has_value()
    );
}

TEST(AuthSampleAuthStoreTests, StoresAuthenticatedUser) {
    AuthStore store;

    store.currentUser.set(
        AuthUser{
            .id = 1,
            .username = "admin",
            .role = "admin"
        }
    );

    ASSERT_TRUE(
        store.currentUser.value()
            .has_value()
    );

    EXPECT_EQ(
        store.currentUser.value()->username,
        "admin"
    );
}

TEST(AuthSampleAuthStoreTests, ClearsAuthenticatedUser) {
    AuthStore store;

    store.currentUser.set(
        AuthUser{
            .id = 1,
            .username = "admin",
            .role = "admin"
        }
    );

    store.currentUser.set(
        std::nullopt
    );

    EXPECT_FALSE(
        store.currentUser.value()
            .has_value()
    );
}

TEST(AuthSampleAuthStoreTests, NotifiesSubscribers) {
    AuthStore store;

    bool notified = false;

    store.currentUser.subscribe(
        [&](const auto&) {
            notified = true;
        }
    );

    store.currentUser.set(
        AuthUser{
            .id = 1,
            .username = "admin",
            .role = "admin"
        }
    );

    EXPECT_TRUE(notified);
}