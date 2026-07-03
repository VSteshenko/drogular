#include "../../examples/portal_demo/providers/graphql/portal_graphql_user_provider.hpp"

#include <drogular/graphql_client.hpp>

#include <gtest/gtest.h>

#include <json/value.h>

TEST(PortalGraphQLUserProviderTests, ReadsUsers) {
    Json::Value users(Json::arrayValue);

    Json::Value admin(Json::objectValue);
    admin["username"] = "admin";
    admin["password"] = "admin";
    admin["role"] = "admin";

    Json::Value user(Json::objectValue);
    user["username"] = "user";
    user["password"] = "user";
    user["role"] = "user";

    users.append(admin);
    users.append(user);

    Json::Value data(Json::objectValue);
    data["users"] = users;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(data);

    PortalGraphQLUserProvider provider(client);

    const auto result = provider.all();

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].username, "admin");
    EXPECT_EQ(result[0].role, "admin");
    EXPECT_EQ(result[1].username, "user");

    ASSERT_EQ(client->requestCount(), 1);
    EXPECT_EQ(
        client->lastRequest()->query(),
        UserQueries::all().toString()
    );
}

TEST(PortalGraphQLUserProviderTests, FindsUserByCredentials) {
    Json::Value user(Json::objectValue);
    user["username"] = "admin";
    user["password"] = "admin";
    user["role"] = "admin";

    Json::Value data(Json::objectValue);
    data["userByCredentials"] = user;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(data);

    PortalGraphQLUserProvider provider(client);

    const auto result =
        provider.findByCredentials("admin", "admin");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->username, "admin");
    EXPECT_EQ(result->role, "admin");

    ASSERT_EQ(client->requestCount(), 1);
    EXPECT_EQ(
        client->lastRequest()->variables()["username"].asString(),
        "admin"
    );
    EXPECT_EQ(
        client->lastRequest()->variables()["password"].asString(),
        "admin"
    );
}

TEST(PortalGraphQLUserProviderTests, ReturnsNulloptForInvalidCredentials) {
    Json::Value data(Json::objectValue);
    data["userByCredentials"] = Json::Value();

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(data);

    PortalGraphQLUserProvider provider(client);

    const auto result =
        provider.findByCredentials("missing", "wrong");

    EXPECT_FALSE(result.has_value());
}

TEST(PortalGraphQLUserProviderTests, CreatesUser) {
    Json::Value created(Json::objectValue);
    created["username"] = "newuser";
    created["password"] = "secret";
    created["role"] = "user";

    Json::Value data(Json::objectValue);
    data["createUser"] = created;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(data);

    PortalGraphQLUserProvider provider(client);

    PortalUser user;
    user.username = "newuser";
    user.password = "secret";
    user.role = "user";

    const auto result = provider.create(user);

    EXPECT_EQ(result.username, "newuser");
    EXPECT_EQ(result.role, "user");

    ASSERT_EQ(client->requestCount(), 1);
    EXPECT_EQ(
        client->lastRequest()->variables()["user"]["username"].asString(),
        "newuser"
    );
}

TEST(PortalGraphQLUserProviderTests, ChecksUserExists) {
    Json::Value users(Json::arrayValue);

    Json::Value admin(Json::objectValue);
    admin["username"] = "admin";
    admin["password"] = "admin";
    admin["role"] = "admin";

    users.append(admin);

    Json::Value data(Json::objectValue);
    data["users"] = users;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(data);

    PortalGraphQLUserProvider provider(client);

    EXPECT_TRUE(provider.exists("admin"));
    EXPECT_FALSE(provider.exists("missing"));
}
