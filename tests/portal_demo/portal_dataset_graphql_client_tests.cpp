#include "../../examples/portal_demo/data/portal_dataset.hpp"
#include "../../examples/portal_demo//providers/graphql/portal_dataset_graphql_client.hpp"
#include "../../examples/portal_demo//providers/graphql/documents/project_queries.hpp"
#include "../../examples/portal_demo//providers/graphql/documents/project_mutations.hpp"
#include "../../examples/portal_demo//providers/graphql/mappers/project_mapper.hpp"
#include "../../examples/portal_demo//providers/graphql/documents/user_queries.hpp"
#include "../../examples/portal_demo//providers/graphql/documents/user_mutations.hpp"
#include "../../examples/portal_demo//providers/graphql/mappers/user_mapper.hpp"

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

TEST(PortalDatasetGraphQLClientTests, ReadsUsersFromDataset) {
    auto dataset =
        std::make_shared<PortalDataset>();

    dataset->addUser({
        .username = "admin",
        .password = "admin",
        .role = "admin"
    });

    PortalDatasetGraphQLClient client(dataset);

    const auto response =
        client.execute(
            UserQueries::all()
        );

    const auto users =
        response.field("users");

    ASSERT_TRUE(users.has_value());
    ASSERT_EQ(users->size(), 1);
    EXPECT_EQ((*users)[0]["username"].asString(), "admin");
}

TEST(PortalDatasetGraphQLClientTests, FindsUserByCredentialsFromDataset) {
    auto dataset =
        std::make_shared<PortalDataset>();

    dataset->addUser({
        .username = "admin",
        .password = "secret",
        .role = "admin"
    });

    PortalDatasetGraphQLClient client(dataset);

    const auto response =
        client.execute(
            UserQueries::findByCredentials(),
            UserMapper::credentialsVariables(
                "admin",
                "secret"
            )
        );

    const auto user =
        response.field("userByCredentials");

    ASSERT_TRUE(user.has_value());
    EXPECT_EQ((*user)["role"].asString(), "admin");
}

TEST(PortalDatasetGraphQLClientTests, ReturnsNullForInvalidCredentials) {
    auto dataset =
        std::make_shared<PortalDataset>();

    PortalDatasetGraphQLClient client(dataset);

    const auto response =
        client.execute(
            UserQueries::findByCredentials(),
            UserMapper::credentialsVariables(
                "missing",
                "wrong"
            )
        );

    const auto user =
        response.field("userByCredentials");

    ASSERT_TRUE(user.has_value());
    EXPECT_TRUE(user->isNull());
}

TEST(PortalDatasetGraphQLClientTests, CreatesUserInDataset) {
    auto dataset =
        std::make_shared<PortalDataset>();

    PortalDatasetGraphQLClient client(dataset);

    PortalUser user;
    user.username = "newuser";
    user.password = "secret";
    user.role = "user";

    const auto response =
        client.execute(
            UserMutations::create(user),
            UserMapper::toVariables(user)
        );

    ASSERT_EQ(dataset->users().size(), 1);
    EXPECT_EQ(dataset->users()[0].username, "newuser");

    const auto created =
        response.field("createUser");

    ASSERT_TRUE(created.has_value());
    EXPECT_EQ((*created)["role"].asString(), "user");
}