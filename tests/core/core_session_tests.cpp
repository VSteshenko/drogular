#include <drogular/session.hpp>

#include <gtest/gtest.h>

TEST(CoreSessionTests, StoresValues) {
    drogular::Session session;

    session.set("user_id", "1");

    const auto value =
        session.get("user_id");

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "1");
}

TEST(CoreSessionTests, ReturnsNulloptForMissingValue) {
    drogular::Session session;

    EXPECT_FALSE(
        session.get("missing").has_value()
    );
}

TEST(CoreSessionTests, RemovesValues) {
    drogular::Session session;

    session.set("user_id", "1");
    session.remove("user_id");

    EXPECT_FALSE(session.has("user_id"));
}

TEST(CoreSessionTests, ClearsValues) {
    drogular::Session session;

    session.set("user_id", "1");
    session.set("role", "admin");

    session.clear();

    EXPECT_FALSE(session.has("user_id"));
    EXPECT_FALSE(session.has("role"));
}