#include <drogular/session_store.hpp>

#include <gtest/gtest.h>

TEST(CoreSessionStoreTests, CreatesSession) {
    drogular::SessionStore store;

    const auto session =
        store.create();

    ASSERT_NE(session, nullptr);

    const auto id =
        session->get("_id");

    ASSERT_TRUE(id.has_value());
    EXPECT_TRUE(store.contains(*id));
}

TEST(CoreSessionStoreTests, GetsExistingSession) {
    drogular::SessionStore store;

    const auto session =
        store.create();

    const auto id =
        session->get("_id");

    ASSERT_TRUE(id.has_value());

    const auto found =
        store.get(*id);

    EXPECT_EQ(found, session);
}

TEST(CoreSessionStoreTests, ReturnsNullForMissingSession) {
    drogular::SessionStore store;

    EXPECT_EQ(
        store.get("missing"),
        nullptr
    );
}

TEST(CoreSessionStoreTests, RemovesSession) {
    drogular::SessionStore store;

    const auto session =
        store.create();

    const auto id =
        session->get("_id");

    ASSERT_TRUE(id.has_value());

    store.remove(*id);

    EXPECT_FALSE(store.contains(*id));
}