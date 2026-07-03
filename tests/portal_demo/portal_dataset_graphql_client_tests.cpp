#include "../../examples/portal_demo/data/portal_dataset.hpp"
#include "../../examples/portal_demo//providers/graphql/portal_dataset_graphql_client.hpp"
#include "../../examples/portal_demo//providers/graphql/documents/project_queries.hpp"
#include "../../examples/portal_demo//providers/graphql/documents/project_mutations.hpp"
#include "../../examples/portal_demo//providers/graphql/mappers/project_mapper.hpp"

#include <gtest/gtest.h>

#include <memory>

TEST(PortalDatasetGraphQLClientTests, FindsProjectByIdFromDataset) {
    auto dataset =
        std::make_shared<PortalDataset>();

    dataset->addProject({
        .id = 1,
        .title = "Customer Portal",
        .status = "active"
    });

    PortalDatasetGraphQLClient client(dataset);

    const auto response =
        client.execute(
            ProjectQueries::findById(1),
            ProjectMapper::idVariables(1)
        );

    const auto project =
        response.field("project");

    ASSERT_TRUE(project.has_value());
    EXPECT_EQ((*project)["title"].asString(), "Customer Portal");
}

TEST(PortalDatasetGraphQLClientTests, CreatesProjectInDataset) {
    auto dataset =
        std::make_shared<PortalDataset>();

    PortalDatasetGraphQLClient client(dataset);

    PortalProject project;
    project.title = "New Project";
    project.status = "active";

    const auto response =
        client.execute(
            ProjectMutations::create(project),
            ProjectMapper::toVariables(project)
        );

    ASSERT_EQ(dataset->projects().size(), 1);
    EXPECT_EQ(dataset->projects()[0].title, "New Project");

    const auto created =
        response.field("createProject");

    ASSERT_TRUE(created.has_value());
    EXPECT_EQ((*created)["id"].asInt(), 1);
}

TEST(PortalDatasetGraphQLClientTests, UpdatesProjectInDataset) {
    auto dataset =
        std::make_shared<PortalDataset>();

    dataset->addProject({
        .id = 1,
        .title = "Old Project",
        .status = "active"
    });

    PortalDatasetGraphQLClient client(dataset);

    PortalProject project;
    project.id = 1;
    project.title = "Updated Project";
    project.status = "done";

    const auto response =
        client.execute(
            ProjectMutations::update(project),
            ProjectMapper::toVariables(project)
        );

    ASSERT_EQ(dataset->projects().size(), 1);
    EXPECT_EQ(dataset->projects()[0].title, "Updated Project");
    EXPECT_EQ(dataset->projects()[0].status, "done");

    const auto updated =
        response.field("updateProject");

    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ((*updated)["title"].asString(), "Updated Project");
}

TEST(PortalDatasetGraphQLClientTests, RemovesProjectFromDataset) {
    auto dataset =
        std::make_shared<PortalDataset>();

    dataset->addProject({
        .id = 1,
        .title = "Project",
        .status = "active"
    });

    PortalDatasetGraphQLClient client(dataset);

    const auto response =
        client.execute(
            ProjectMutations::remove(1),
            ProjectMapper::idVariables(1)
        );

    EXPECT_TRUE(dataset->projects().empty());

    const auto removed =
        response.field("removeProject");

    ASSERT_TRUE(removed.has_value());
    EXPECT_TRUE(removed->asBool());
}

TEST(PortalDatasetGraphQLClientTests, ReturnsNullForMissingProject) {
    auto dataset =
        std::make_shared<PortalDataset>();

    PortalDatasetGraphQLClient client(dataset);

    const auto response =
        client.execute(
            ProjectQueries::findById(404),
            ProjectMapper::idVariables(404)
        );

    const auto project =
        response.field("project");

    ASSERT_TRUE(project.has_value());
    EXPECT_TRUE(project->isNull());
}