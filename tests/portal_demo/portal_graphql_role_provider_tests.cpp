#include "providers/graphql/portal_graphql_role_provider.hpp"

#include <drogular/static_graphql_client.hpp>

#include <gtest/gtest.h>

#include <json/value.h>

TEST(PortalGraphQLRoleProviderTests, CreatesRole) {
    Json::Value created(Json::objectValue);

    created["id"] = 3;
    created["code"] = "manager";
    created["title"] = "Manager";

    Json::Value data(Json::objectValue);
    data["createRole"] = created;

    auto client =
        std::make_shared<
            drogular::StaticGraphQLClient
        >(data);

    PortalGraphQLRoleProvider provider(client);

    PortalRoleCreate input;
    input.code = "manager";
    input.title = "Manager";

    const auto result =
        provider.create(input);

    EXPECT_EQ(result.id, 3);
    EXPECT_EQ(result.code, "manager");
    EXPECT_EQ(result.title, "Manager");

    ASSERT_EQ(
        client->requestCount(),
        1
    );

    EXPECT_EQ(
        client
            ->lastRequest()
            ->variables()["role"]["code"]
            .asString(),
        "manager"
    );
}

TEST(PortalGraphQLRoleProviderTests, SendsOnlyProvidedRoleUpdateFields) {
    Json::Value updated(Json::objectValue);

    updated["id"] = 3;
    updated["code"] = "manager";
    updated["title"] = "Project Manager";

    Json::Value data(Json::objectValue);
    data["updateRole"] = updated;

    auto client =
        std::make_shared<
            drogular::StaticGraphQLClient
        >(data);

    PortalGraphQLRoleProvider provider(client);

    PortalRoleUpdate input;
    input.id = 3;
    input.title = "Project Manager";

    const auto result =
        provider.update(input);

    EXPECT_EQ(result.id, 3);
    EXPECT_EQ(
        result.title,
        "Project Manager"
    );

    ASSERT_EQ(
        client->requestCount(),
        1
    );

    const auto variables =
        client
            ->lastRequest()
            ->variables()["role"];

    EXPECT_EQ(
        variables["id"].asInt(),
        3
    );

    EXPECT_EQ(
        variables["title"].asString(),
        "Project Manager"
    );

    EXPECT_FALSE(
        variables.isMember("code")
    );
}

TEST(PortalGraphQLRoleProviderTests, RemovesRole) {
    Json::Value data(Json::objectValue);
    data["removeRole"] = true;

    auto client =
        std::make_shared<
            drogular::StaticGraphQLClient
        >(data);

    PortalGraphQLRoleProvider provider(client);

    EXPECT_TRUE(
        provider.remove(3)
    );

    ASSERT_EQ(
        client->requestCount(),
        1
    );

    EXPECT_EQ(
        client
            ->lastRequest()
            ->variables()["id"]
            .asInt(),
        3
    );
}

TEST(PortalGraphQLRoleProviderTests, ReturnsFalseWhenRoleCannotBeRemoved) {
    Json::Value data(Json::objectValue);
    data["removeRole"] = false;

    auto client =
        std::make_shared<
            drogular::StaticGraphQLClient
        >(data);

    PortalGraphQLRoleProvider provider(client);

    EXPECT_FALSE(
        provider.remove(1)
    );
}

TEST(PortalGraphQLRoleProviderTests, FindsRoleById) {
    Json::Value role(Json::objectValue);
    role["id"] = 2;
    role["code"] = "user";
    role["title"] = "User";

    Json::Value data(Json::objectValue);
    data["role"] = role;

    auto client =
        std::make_shared<
            drogular::StaticGraphQLClient
        >(data);

    PortalGraphQLRoleProvider provider(client);

    const auto result =
        provider.findById(2);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, 2);
    EXPECT_EQ(result->code, "user");
    EXPECT_EQ(result->title, "User");
}