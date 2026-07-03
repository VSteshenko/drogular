#include "../../examples/portal_demo/providers/graphql/portal_graphql_project_provider.hpp"

#include <drogular/graphql_client.hpp>

#include <gtest/gtest.h>

#include <json/value.h>

TEST(PortalGraphQLProjectProviderTests, ReadsProjects) {
    Json::Value projects(Json::arrayValue);

    Json::Value first(Json::objectValue);
    first["id"] = 1;
    first["title"] = "Customer Portal";
    first["status"] = "active";

    Json::Value second(Json::objectValue);
    second["id"] = 2;
    second["title"] = "Internal Dashboard";
    second["status"] = "paused";

    projects.append(first);
    projects.append(second);

    Json::Value data(Json::objectValue);
    data["projects"] = projects;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    PortalGraphQLProjectProvider provider(
        client
    );

    const auto result =
        provider.all();

    ASSERT_EQ(result.size(), 2);

    EXPECT_EQ(result[0].id, 1);
    EXPECT_EQ(result[0].title, "Customer Portal");
    EXPECT_EQ(result[0].status, "active");

    EXPECT_EQ(result[1].id, 2);
    EXPECT_EQ(result[1].title, "Internal Dashboard");
    EXPECT_EQ(result[1].status, "paused");
}

TEST(PortalGraphQLProjectProviderTests, FindsProjectById) {
    Json::Value project(Json::objectValue);
    project["id"] = 5;
    project["title"] = "Portal Demo";
    project["status"] = "done";

    Json::Value data(Json::objectValue);
    data["project"] = project;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    PortalGraphQLProjectProvider provider(
        client
    );

    const auto result =
        provider.findById(5);

    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->id, 5);
    EXPECT_EQ(result->title, "Portal Demo");
    EXPECT_EQ(result->status, "done");
}

TEST(PortalGraphQLProjectProviderTests, ReturnsNulloptForMissingProject) {
    Json::Value data(Json::objectValue);
    data["project"] = Json::Value();

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    PortalGraphQLProjectProvider provider(
        client
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

    Json::Value data(Json::objectValue);
    data["createProject"] = created;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    PortalGraphQLProjectProvider provider(
        client
    );

    PortalProject project;
    project.title = "New Project";
    project.status = "active";

    const auto result =
        provider.create(project);

    EXPECT_EQ(result.id, 10);
    EXPECT_EQ(result.title, "New Project");
    EXPECT_EQ(result.status, "active");
}

TEST(PortalGraphQLProjectProviderTests, UpdatesProject) {
    Json::Value updated(Json::objectValue);
    updated["id"] = 10;
    updated["title"] = "Updated Project";
    updated["status"] = "done";

    Json::Value data(Json::objectValue);
    data["updateProject"] = updated;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    PortalGraphQLProjectProvider provider(
        client
    );

    PortalProject project;
    project.id = 10;
    project.title = "Updated Project";
    project.status = "done";

    EXPECT_TRUE(
        provider.update(project)
    );
}

TEST(PortalGraphQLProjectProviderTests, RemovesProject) {
    Json::Value data(Json::objectValue);
    data["removeProject"] = true;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            data
        );

    PortalGraphQLProjectProvider provider(
        client
    );

    EXPECT_TRUE(
        provider.remove(10)
    );
}