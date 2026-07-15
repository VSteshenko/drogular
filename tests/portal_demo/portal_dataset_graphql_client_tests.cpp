#include "../../examples/portal_demo/data/portal_dataset.hpp"
#include "../../examples/portal_demo/data/demo_dataset.hpp"
#include "../../examples/portal_demo/providers/graphql/portal_dataset_graphql_client.hpp"
#include "../../examples/portal_demo/providers/graphql/documents/project_queries.hpp"
#include "../../examples/portal_demo/providers/graphql/documents/project_mutations.hpp"
#include "../../examples/portal_demo/providers/graphql/mappers/project_mapper.hpp"
#include "../../examples/portal_demo/providers/graphql/documents/user_queries.hpp"
#include "../../examples/portal_demo/providers/graphql/documents/user_mutations.hpp"
#include "../../examples/portal_demo/providers/graphql/mappers/user_mapper.hpp"
#include "../../examples/portal_demo/providers/graphql/documents/role_mutations.hpp"
#include "../../examples/portal_demo/providers/graphql/mappers/role_mapper.hpp"
#include "../../examples/portal_demo/providers/graphql/portal_graphql_user_provider.hpp"
#include "../../examples/portal_demo/providers/graphql/portal_graphql_project_provider.hpp"
#include "../../examples/portal_demo/providers/graphql/portal_graphql_project_type_provider.hpp"

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
    project.ownerId = 1;

    const auto response =
        client.execute(
            ProjectMutations::create(project),
            ProjectMapper::toVariables(project)
        );

    ASSERT_EQ(dataset->projects().size(), 1);
    EXPECT_EQ(dataset->projects()[0].title, "New Project");
    EXPECT_EQ(dataset->projects()[0].status, "active");
    EXPECT_EQ(dataset->projects()[0].ownerId, 1);

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
        .status = "active",
        .ownerId = 1
    });

    PortalDatasetGraphQLClient client(dataset);

    PortalProjectUpdate project;
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
    EXPECT_EQ(dataset->projects()[0].ownerId, 1);

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
        .status = "active",
        .ownerId = 1
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

//TODO: Update when password change action will be ready
TEST(PortalDatasetGraphQLClientTests, UpdatesUserInDataset) {
    auto dataset =
        std::make_shared<PortalDataset>();

    dataset->addUser({
        .id = 2,
        .username = "user",
        .password = "user",
        .role = "user"
    });

    PortalDatasetGraphQLClient client(dataset);

    PortalUserUpdate user;
    user.id = 2;
    user.username = "user";
    user.role = "admin";

    const auto response =
        client.execute(
            UserMutations::update(user),
            UserMapper::toVariables(user)
        );

    ASSERT_EQ(dataset->users().size(), 1);
    // EXPECT_EQ(dataset->users()[0].password, "newpass");
    EXPECT_EQ(dataset->users()[0].role, "admin");

    const auto updated =
        response.field("updateUser");

    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ((*updated)["role"].asString(), "admin");
}

TEST(PortalDatasetGraphQLClientTests, CreatesProjectType) {
    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    auto client =
        std::make_shared<PortalDatasetGraphQLClient>(
            dataset
        );

    PortalGraphQLProjectTypeProvider provider(client);

    PortalProjectTypeCreate input;
    input.code = "support";
    input.title = "Support";

    const auto created =
        provider.create(input);

    EXPECT_GT(created.id, 0);
    EXPECT_EQ(created.code, "support");
    EXPECT_EQ(created.title, "Support");
    EXPECT_EQ(dataset->projectTypes().back().code, "support");
}

TEST(PortalDatasetGraphQLClientTests, UpdatesOnlyProvidedProjectTypeFields ) {
    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    auto client =
        std::make_shared<PortalDatasetGraphQLClient>(
            dataset
        );

    PortalGraphQLProjectTypeProvider provider(client);

    const auto before =
        dataset->projectTypes().front();

    PortalProjectTypeUpdate input;
    input.id = before.id;
    input.title = "Updated type";

    const auto updated =
        provider.update(input);

    EXPECT_EQ(updated.title, "Updated type");
    EXPECT_EQ(updated.code, before.code);
}

TEST(PortalDatasetGraphQLClientTests, RejectsRemovingUsedProjectType) {
    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    auto client =
        std::make_shared<PortalDatasetGraphQLClient>(
            dataset
        );

    PortalGraphQLProjectTypeProvider provider(client);

    const auto usedId =
        dataset->projects().front().projectTypeId;

    EXPECT_FALSE(
        provider.remove(usedId)
    );
}

TEST(PortalDatasetGraphQLClientTests, RemovesUnusedProjectType) {
    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    dataset->addProjectType({
        .id = 99,
        .code = "unused",
        .title = "Unused"
    });

    auto client =
        std::make_shared<PortalDatasetGraphQLClient>(
            dataset
        );

    PortalGraphQLProjectTypeProvider provider(client);

    EXPECT_TRUE(
        provider.remove(99)
    );

    EXPECT_TRUE(
        std::none_of(
            dataset->projectTypes().begin(),
            dataset->projectTypes().end(),
            [](const PortalProjectType& type) {
                return type.id == 99;
            }
        )
    );
}

TEST(PortalDatasetGraphQLClientTests, CreatesRoleInDataset) {
    auto dataset =
        std::make_shared<PortalDataset>();

    PortalDatasetGraphQLClient client(dataset);

    PortalRoleCreate input;
    input.code = "manager";
    input.title = "Manager";

    const auto response =
        client.execute(
            RoleMutations::create(input),
            RoleMapper::toVariables(input)
        );

    ASSERT_EQ(
        dataset->roles().size(),
        1
    );

    EXPECT_EQ(
        dataset->roles()[0].id,
        1
    );

    EXPECT_EQ(
        dataset->roles()[0].code,
        "manager"
    );

    EXPECT_EQ(
        dataset->roles()[0].title,
        "Manager"
    );

    const auto created =
        response.field("createRole");

    ASSERT_TRUE(
        created.has_value()
    );

    EXPECT_EQ(
        (*created)["id"].asInt(),
        1
    );

    EXPECT_EQ(
        (*created)["code"].asString(),
        "manager"
    );
}

TEST(PortalDatasetGraphQLClientTests, UpdatesOnlyProvidedRoleFields) {
    auto dataset =
        std::make_shared<PortalDataset>();

    dataset->addRole({
        .id = 1,
        .code = "manager",
        .title = "Manager"
    });

    PortalDatasetGraphQLClient client(dataset);

    PortalRoleUpdate input;
    input.id = 1;
    input.title = "Project Manager";

    const auto response =
        client.execute(
            RoleMutations::update(input),
            RoleMapper::toVariables(input)
        );

    ASSERT_EQ(
        dataset->roles().size(),
        1
    );

    EXPECT_EQ(
        dataset->roles()[0].code,
        "manager"
    );

    EXPECT_EQ(
        dataset->roles()[0].title,
        "Project Manager"
    );

    const auto updated =
        response.field("updateRole");

    ASSERT_TRUE(
        updated.has_value()
    );

    EXPECT_EQ(
        (*updated)["title"].asString(),
        "Project Manager"
    );
}

TEST(PortalDatasetGraphQLClientTests, RemovesUnusedRole) {
    auto dataset =
        std::make_shared<PortalDataset>();

    dataset->addRole({
        .id = 1,
        .code = "manager",
        .title = "Manager"
    });

    PortalDatasetGraphQLClient client(dataset);

    const auto response =
        client.execute(
            RoleMutations::remove(),
            RoleMapper::idVariables(1)
        );

    EXPECT_TRUE(
        dataset->roles().empty()
    );

    const auto removed =
        response.field("removeRole");

    ASSERT_TRUE(
        removed.has_value()
    );

    EXPECT_TRUE(
        removed->asBool()
    );
}

TEST(PortalDatasetGraphQLClientTests, RejectsRemovingUsedRole) {
    auto dataset =
        std::make_shared<PortalDataset>();

    dataset
        ->addRole({
            .id = 1,
            .code = "admin",
            .title = "Administrator"
        })
        .addUser({
            .id = 1,
            .username = "admin",
            .password = "secret",
            .role = "admin"
        });

    PortalDatasetGraphQLClient client(dataset);

    const auto response =
        client.execute(
            RoleMutations::remove(),
            RoleMapper::idVariables(1)
        );

    ASSERT_EQ(
        dataset->roles().size(),
        1
    );

    const auto removed =
        response.field("removeRole");

    ASSERT_TRUE(
        removed.has_value()
    );

    EXPECT_FALSE(
        removed->asBool()
    );
}

TEST(PortalDatasetGraphQLClientTests, SearchesProjectsByPartialTitle) {
    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    auto users =
        std::make_shared<PortalGraphQLUserProvider>(
            std::make_shared<PortalDatasetGraphQLClient>(
                dataset
            )
        );

    PortalGraphQLProjectProvider provider(
        std::make_shared<PortalDatasetGraphQLClient>(
            dataset
        ),
        users
    );

    PortalProjectFilter filter;
    filter.search = "portal";

    const auto result =
        provider.search(filter);

    ASSERT_FALSE(result.empty());

    for (const auto& project : result) {
        auto title = project.title;

        std::transform(
            title.begin(),
            title.end(),
            title.begin(),
            [](unsigned char character) {
                return static_cast<char>(
                    std::tolower(character)
                );
            }
        );

        EXPECT_NE(
            title.find("portal"),
            std::string::npos
        );
    }
}

TEST(PortalDatasetGraphQLClientTests, ProjectSearchIsCaseInsensitive) {
    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    auto users =
        std::make_shared<PortalGraphQLUserProvider>(
            std::make_shared<PortalDatasetGraphQLClient>(
                dataset
            )
        );

    PortalGraphQLProjectProvider provider(
        std::make_shared<PortalDatasetGraphQLClient>(
            dataset
        ),
        users
    );

    PortalProjectFilter filter;
    filter.search = "PoRtAl";

    const auto result =
        provider.search(filter);

    ASSERT_FALSE(result.empty());

    for (const auto& project : result) {
        auto title = project.title;

        std::transform(
            title.begin(),
            title.end(),
            title.begin(),
            [](unsigned char character) {
                return static_cast<char>(
                    std::tolower(character)
                );
            }
        );

        EXPECT_NE(
            title.find("portal"),
            std::string::npos
        );
    }
}

TEST(PortalDatasetGraphQLClientTests, FiltersProjectsByStatus) {
    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    auto client =
        std::make_shared<
            PortalDatasetGraphQLClient
        >(dataset);

    auto users =
        std::make_shared<
            PortalGraphQLUserProvider
        >(client);

    PortalGraphQLProjectProvider provider(
        client,
        users
    );

    PortalProjectFilter filter;
    filter.status = "active";

    const auto result =
        provider.search(filter);

    ASSERT_FALSE(result.empty());

    for (const auto& project : result) {
        EXPECT_EQ(
            project.status,
            "active"
        );
    }
}

TEST(PortalDatasetGraphQLClientTests, CombinesProjectSearchAndStatusFilter) {
    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    const auto source =
        dataset->projects().front();

    dataset->addProject({
        .id = 999,
        .title = source.title + " Copy",
        .status = "done",
        .ownerId = source.ownerId,
        .projectTypeId =
            source.projectTypeId
    });

    auto client =
        std::make_shared<
            PortalDatasetGraphQLClient
        >(dataset);

    auto users =
        std::make_shared<
            PortalGraphQLUserProvider
        >(client);

    PortalGraphQLProjectProvider provider(
        client,
        users
    );

    PortalProjectFilter filter;
    filter.search = source.title;
    filter.status = source.status;

    const auto result =
        provider.search(filter);

    ASSERT_FALSE(result.empty());

    for (const auto& project : result) {
        EXPECT_EQ(
            project.status,
            source.status
        );

        EXPECT_NE(
            lower(project.title).find(
                lower(source.title)
            ),
            std::string::npos
        );
    }

    EXPECT_TRUE(
        std::none_of(
            result.begin(),
            result.end(),
            [](const PortalProject& project) {
                return project.id == 999;
            }
        )
    );
}

TEST(PortalDatasetGraphQLClientTests, FiltersProjectsByProjectType) {
    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    const auto expectedTypeId =
        dataset->projects()
            .front()
            .projectTypeId;

    auto client =
        std::make_shared<
            PortalDatasetGraphQLClient
        >(dataset);

    auto users =
        std::make_shared<
            PortalGraphQLUserProvider
        >(client);

    PortalGraphQLProjectProvider provider(
        client,
        users
    );

    PortalProjectFilter filter;
    filter.projectTypeId =
        expectedTypeId;

    const auto result =
        provider.search(filter);

    ASSERT_FALSE(result.empty());

    for (const auto& project : result) {
        EXPECT_EQ(
            project.projectTypeId,
            expectedTypeId
        );
    }
}

TEST(PortalDatasetGraphQLClientTests, CombinesProjectSearchStatusAndTypeFilter) {
    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    const auto expected =
        dataset->projects().front();

    dataset->addProject({
        .id = 999,
        .title = expected.title + " Copy",
        .status = expected.status,
        .ownerId = expected.ownerId,
        .projectTypeId =
            expected.projectTypeId == 1
                ? 2
                : 1
    });

    auto client =
        std::make_shared<
            PortalDatasetGraphQLClient
        >(dataset);

    auto users =
        std::make_shared<
            PortalGraphQLUserProvider
        >(client);

    PortalGraphQLProjectProvider provider(
        client,
        users
    );

    PortalProjectFilter filter;
    filter.search = expected.title;
    filter.status = expected.status;
    filter.projectTypeId =
        expected.projectTypeId;

    const auto result =
        provider.search(filter);

    ASSERT_FALSE(result.empty());

    for (const auto& project : result) {
        EXPECT_EQ(
            project.status,
            expected.status
        );

        EXPECT_EQ(
            project.projectTypeId,
            expected.projectTypeId
        );
    }

    EXPECT_TRUE(
        std::none_of(
            result.begin(),
            result.end(),
            [](const PortalProject& project) {
                return project.id == 999;
            }
        )
    );
}