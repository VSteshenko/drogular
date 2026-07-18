#include "features/users/graphql/portal_graphql_user_provider.hpp"

#include <drogular/static_graphql_client.hpp>

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

    PortalUserCreate user;
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

//TODO: Update when password change action will be ready
TEST(PortalGraphQLUserProviderTests, UpdatesUser) {
    Json::Value updated(Json::objectValue);
    updated["id"] = 2;
    updated["username"] = "user";
    updated["password"] = "newpass";
    updated["role"] = "admin";

    Json::Value data(Json::objectValue);
    data["updateUser"] = updated;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(data);

    PortalGraphQLUserProvider provider(client);

    PortalUserUpdate user;
    user.id = 2;
    user.username = "user";
    user.role = "admin";

    EXPECT_EQ(
        provider.update(user).id,
        2
    );

    ASSERT_EQ(client->requestCount(), 1);
    EXPECT_EQ(
        client->lastRequest()->variables()["user"]["id"].asInt(),
        2
    );
}
TEST(PortalGraphQLUserProviderTests, SearchesUsersWithQueryState) {
    Json::Value items(Json::arrayValue);
    Json::Value admin(Json::objectValue);
    admin["id"] = 1;
    admin["username"] = "admin";
    admin["password"] = "secret";
    admin["role"] = "admin";
    items.append(admin);

    Json::Value page(Json::objectValue);
    page["items"] = items;
    page["page"] = 2;
    page["pageSize"] = 1;
    page["totalItems"] = 2;
    page["totalPages"] = 2;

    Json::Value data(Json::objectValue);
    data["userPage"] = page;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(data);
    PortalGraphQLUserProvider provider(client);

    PortalUserQuery query;
    query.search = "adm";
    query.role = "admin";
    query.sorting.push_back({
        .field = "role",
        .direction = PortalUserSortDirection::Descending
    });
    query.page = 2;
    query.pageSize = 1;

    const auto result = provider.search(query);

    ASSERT_EQ(result.items.size(), 1);
    EXPECT_EQ(result.items.front().username, "admin");
    EXPECT_EQ(result.page, 2);
    EXPECT_EQ(result.totalPages, 2);
    ASSERT_EQ(client->requestCount(), 1);
    EXPECT_EQ(
        client->lastRequest()->variables()["search"].asString(),
        "adm"
    );
    EXPECT_EQ(
        client->lastRequest()->variables()["sorting"][0]["field"].asString(),
        "role"
    );
}