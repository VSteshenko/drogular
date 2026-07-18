#include "features/projects/graphql/portal_graphql_project_provider.hpp"
#include "providers/graphql/portal_graphql_user_provider.hpp"

#include <drogular/static_graphql_client.hpp>

#include <gtest/gtest.h>

#include <json/value.h>

TEST(PortalGraphQLProjectProviderTests, ReadsProjects) {
    Json::Value projects(Json::arrayValue);

    Json::Value first(Json::objectValue);
    first["id"] = 1;
    first["title"] = "Customer Portal";
    first["status"] = "active";
    first["ownerId"] = 1;
    first["projectTypeId"] = 1;

    Json::Value second(Json::objectValue);
    second["id"] = 2;
    second["title"] = "Internal Dashboard";
    second["status"] = "paused";
    second["ownerId"] = 2;
    second["projectTypeId"] = 2;

    projects.append(first);
    projects.append(second);

    Json::Value data(Json::objectValue);
    data["projects"] = projects;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    auto userProvider =
        std::make_shared<PortalGraphQLUserProvider>(
            client
        );

    PortalGraphQLProjectProvider provider(
        client,
        userProvider
    );

    const auto result = provider.all();

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].title, "Customer Portal");
    EXPECT_EQ(result[1].title, "Internal Dashboard");

    ASSERT_EQ(client->requestCount(), 1);

    EXPECT_EQ(
        client->lastRequest()->query(),
        ProjectQueries::all().toString()
    );
}

TEST(PortalGraphQLProjectProviderTests, FindsProjectById) {
    Json::Value project(Json::objectValue);
    project["id"] = 5;
    project["title"] = "Portal Demo";
    project["status"] = "done";
    project["ownerId"] = 1;
    project["projectTypeId"] = 1;

    Json::Value data(Json::objectValue);
    data["project"] = project;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    auto userProvider =
        std::make_shared<PortalGraphQLUserProvider>(
            client
        );

    PortalGraphQLProjectProvider provider(
        client,
        userProvider
    );

    const auto result = provider.findById(5);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, 5);
    EXPECT_EQ(result->title, "Portal Demo");
    EXPECT_EQ(result->status, "done");
    EXPECT_EQ(result->ownerId, 1);
    EXPECT_EQ(result->projectTypeId, 1);

    ASSERT_EQ(client->requestCount(), 1);

    EXPECT_EQ(
        client->lastRequest()->variables()["id"].asInt(),
        5
    );
}

TEST(PortalGraphQLProjectProviderTests, ReturnsNulloptForMissingProject) {
    Json::Value data(Json::objectValue);
    data["project"] = Json::Value();

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    auto userProvider =
        std::make_shared<PortalGraphQLUserProvider>(
            client
        );

    PortalGraphQLProjectProvider provider(
        client,
        userProvider
    );

    const auto result =
        provider.findById(404);

    EXPECT_FALSE(result.has_value());
}

TEST(PortalGraphQLProjectProviderTests, CreatesProject) {
    Json::Value created(Json::objectValue);
    created["id"] = 10;
    created["title"] = "New Project";
    created["status"] = "active";
    created["ownerId"] = 1;
    created["projectTypeId"] = 1;

    Json::Value data(Json::objectValue);
    data["createProject"] = created;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    auto userProvider =
        std::make_shared<PortalGraphQLUserProvider>(
            client
        );

    PortalGraphQLProjectProvider provider(
        client,
        userProvider
    );

    PortalProjectCreate project;
    project.title = "New Project";
    project.status = "active";
    project.projectTypeId = 1;

    const auto result = provider.create(
        project,
        1
    );

    EXPECT_EQ(result.id, 10);
    EXPECT_EQ(result.title, "New Project");
    EXPECT_EQ(result.status, "active");
    EXPECT_EQ(result.ownerId, 1);
    EXPECT_EQ(result.projectTypeId, 1);

    ASSERT_EQ(client->requestCount(), 1);

    EXPECT_EQ(
        client->lastRequest()->variables()["project"]["title"].asString(),
        "New Project"
    );
}

TEST(PortalGraphQLProjectProviderTests, UpdatesProject) {
    Json::Value updated(Json::objectValue);
    updated["id"] = 10;
    updated["title"] = "Updated Project";
    updated["status"] = "done";
    updated["ownerId"] = 1;
    updated["projectTypeId"] = 1;

    Json::Value data(Json::objectValue);
    data["updateProject"] = updated;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    auto userProvider =
        std::make_shared<PortalGraphQLUserProvider>(
            client
        );

    PortalGraphQLProjectProvider provider(
        client,
        userProvider
    );

    PortalProjectUpdate project;
    project.id = 10;
    project.title = "Updated Project";
    project.status = "done";
    project.projectTypeId = 1;

    EXPECT_EQ(
        provider.update(project).id,
        10
    );

    ASSERT_EQ(client->requestCount(), 1);

    EXPECT_EQ(
        client->lastRequest()->variables()["project"]["id"].asInt(),
        10
    );
}

TEST(PortalGraphQLProjectProviderTests, RemovesProject) {
    Json::Value data(Json::objectValue);
    data["removeProject"] = true;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    auto userProvider =
        std::make_shared<PortalGraphQLUserProvider>(
            client
        );

    PortalGraphQLProjectProvider provider(
        client,
        userProvider
    );

    EXPECT_TRUE(provider.remove(10));

    ASSERT_EQ(client->requestCount(), 1);

    EXPECT_EQ(
        client->lastRequest()->variables()["id"].asInt(),
        10
    );
}