#include "portal_application_test_host.hpp"
#include "../support/html_test_support.hpp"

#include "../../examples/portal_demo/actions/create_project_action.hpp"
#include "../../examples/portal_demo/data/demo_dataset.hpp"
#include "../../examples/portal_demo/actions/create_user_action.hpp"
#include "../../examples/portal_demo/actions/update_user_action.hpp"
#include "../../examples/portal_demo/actions/update_project_action.hpp"
#include "../../examples/portal_demo/actions/delete_project_action.hpp"
#include "../../examples/portal_demo/pages/projects_page.hpp"
#include "../../examples/portal_demo/pages/project_edit_page.hpp"
#include "../../examples/portal_demo/pages/users_page.hpp"
#include "../../examples/portal_demo/pages/user_edit_page.hpp"
#include "../../examples/portal_demo/pages/project_types_page.hpp"
#include "../../examples/portal_demo/pages/project_type_edit_page.hpp"
#include "../../examples/portal_demo/actions/create_project_type_action.hpp"
#include "../../examples/portal_demo/actions/update_project_type_action.hpp"
#include "../../examples/portal_demo/actions/delete_project_type_action.hpp"

#include <algorithm>

#include <gtest/gtest.h>

TEST(PortalApplicationTests, AdminCreatesProject) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result =
        app.post<PortalCreateProjectAction>({
            {"title", "Application Test Project"},
            {"projectTypeId", "2"},
            {"status", "active"}
        });

    EXPECT_EQ(
        result.location(),
        "/projects?success=project_created"
    );

    ASSERT_EQ(
        app.projectCount(),
        3
    );

    const auto project =
        app.dataset().projects().back();

    EXPECT_EQ(
        project.title,
        "Application Test Project"
    );

    EXPECT_EQ(
        project.status,
        "active"
    );

    EXPECT_EQ(
        project.projectTypeId,
        2
    );

    EXPECT_EQ(
        project.ownerId,
        1
    );
}

TEST(PortalApplicationTests, UsesDefaultStatusWhenCreateStatusIsMissing) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    app.execute<PortalCreateProjectAction>({
        {"title", "Default status project"},
        {"projectTypeId", "1"}
    });

    const auto project =
        app.dataset().projects().back();

    EXPECT_EQ(
        project.status,
        "active"
    );
}

TEST(PortalApplicationTests, GuestCannotCreateProject) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto result =
        app.post<PortalCreateProjectAction>({
            {"title", "Guest Project"},
            {"projectTypeId", "2"},
            {"status", "active"}
        });

    EXPECT_EQ(
        result.location(),
        "/login"
    );

    EXPECT_EQ(
        app.projectCount(),
        2
    );
}

TEST(PortalApplicationTests, AdminCreatesUser) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result =
        app.post<PortalCreateUserAction>({
            {"username", "new-user"},
            {"password", "secret"},
            {"role", "user"}
        });

    EXPECT_EQ(
        result.location(),
        "/users?success=user_created"
    );

    EXPECT_EQ(
        app.userCount(),
        3
    );

    const auto& user =
        app.dataset().users().back();

    EXPECT_EQ(user.username, "new-user");
    EXPECT_EQ(user.password, "secret");
    EXPECT_EQ(user.role, "user");
    EXPECT_GT(user.id, 0);
}

TEST(PortalApplicationTests, GuestCannotCreateUser) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto result =
        app.post<PortalCreateUserAction>({
            {"username", "john"},
            {"password", "secret"},
            {"role", "user"}
        });

    EXPECT_EQ(
        result.location(),
        "/login"
    );

    EXPECT_EQ(
        app.userCount(),
        2
    );
}

TEST(PortalApplicationTests, UserCannotCreateUser) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsUser();

    const auto result =
        app.post<PortalCreateUserAction>({
            {"username", "john"},
            {"password", "secret"},
            {"role", "user"}
        });

    EXPECT_EQ(
        result.location(),
        "/users?error=access_denied"
    );

    EXPECT_EQ(
        app.userCount(),
        2
    );
}

TEST(PortalApplicationTests, UpdatesOnlyProvidedUserFields) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto before =
        app.dataset().users().front();

    const auto result =
        app.execute<PortalUpdateUserAction>(
            {
                {"role", "user"}
            },
            {
                {"id", std::to_string(before.id)}
            }
        );

    EXPECT_EQ(
        result.location(),
        "/users?success=user_updated"
    );

    const auto& after =
        app.dataset().users().front();

    EXPECT_EQ(after.role, "user");
    EXPECT_EQ(after.username, before.username);
    EXPECT_EQ(after.password, before.password);
}

TEST(PortalApplicationTests, UpdatesOnlyProvidedProjectFields) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto before =
        app.dataset().projects().front();

    const auto result =
        app.execute<PortalUpdateProjectAction>(
            {
                {"title", "Updated title"}
            },
            {
                {"id", std::to_string(before.id)}
            }
        );

    EXPECT_EQ(
        result.location(),
        "/projects/1?success=project_updated"
    );

    const auto& after =
        app.dataset().projects().front();

    EXPECT_EQ(
        after.title,
        "Updated title"
    );

    EXPECT_EQ(
        after.status,
        before.status
    );

    EXPECT_EQ(
        after.projectTypeId,
        before.projectTypeId
    );

    EXPECT_EQ(
        after.ownerId,
        before.ownerId
    );
}

TEST(PortalApplicationTests, AdminDeletesProject) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result =
        app.post<PortalDeleteProjectAction>(
            {},
            {
                {"id", "1"}
            }
        );

    EXPECT_EQ(
        result.location(),
        "/projects?success=project_deleted"
    );

    EXPECT_EQ(
        app.projectCount(),
        1
    );
}

TEST(PortalApplicationTests, ProjectEditSelectsCurrentProjectType) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectEditPage>(
            {},
            {
                {"id", "1"}
            }
        );

    EXPECT_NE(
        html.find(
            R"(option value="1" selected)"
        ),
        std::string::npos
    );

    EXPECT_NE(
        html.find(
            R"(option value="2")"
        ),
        std::string::npos
    );
}

TEST(PortalApplicationTests, ProjectEditRequiresAuthentication) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto html =
        app.render<PortalProjectEditPage>(
            {},
            {
                {"id", "1"}
            }
        );

    EXPECT_NE(
        html.find("/login"),
        std::string::npos
    );
}

TEST(PortalApplicationTests, ProjectCreateFormUsesRequiredSchemaMetadata) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectsPage>();

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="title")",
            "required"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="status")",
            "required"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="projectTypeId")",
            "required"
        )
    );
}

TEST(PortalApplicationTests, ProjectEditFormUsesRequiredSchemaMetadata) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectEditPage>(
            {},
            {
                {"id", "1"}
            }
        );

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="title")",
            "required"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="status")",
            "required"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="projectTypeId")",
            "required"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::optionSelected(
            html,
            "1"
        )
    );
}

TEST(PortalApplicationTests, UserCreateFormBuildsRoleSelectFromProvider) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalUsersPage>();

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="role")",
            "required"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsOption(
            html,
            "admin"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsOption(
            html,
            "user"
        )
    );
}

TEST(PortalApplicationTests, UserCreateFormRendersAllDatasetRoles) {
    auto dataset =
        DemoDataset::create();

    dataset.addRole({
        .id = 3,
        .code = "manager",
        .title = "Manager"
    });

    PortalApplicationTestHost app(
        std::move(dataset)
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalUsersPage>();

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="role")",
            "required"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsOption(
            html,
            "admin"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsOption(
            html,
            "user"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "Manager"
        )
    );
}

TEST(PortalApplicationTests, AdminRendersUserEditForm) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto user =
        app.dataset().users().back();

    const auto html =
        app.render<PortalUserEditPage>(
            {},
            {
                {
                    "id",
                    std::to_string(user.id)
                }
            }
        );

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="username")",
            "required"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="role")",
            "required"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::optionSelected(
            html,
            user.role
        )
    );

    const auto username =
        HtmlTestSupport::attributeValue(
            html,
            R"(name="username")",
            "value"
        );

    ASSERT_TRUE(
        username.has_value()
    );

    EXPECT_EQ(
        *username,
        user.username
    );
}

TEST(PortalApplicationTests, UserCannotRenderUserEditForm) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsUser();

    const auto user =
        app.dataset().users().front();

    const auto html =
        app.render<PortalUserEditPage>(
            {},
            {
                {
                    "id",
                    std::to_string(user.id)
                }
            }
        );

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "Access denied"
        )
    );

    EXPECT_FALSE(
        HtmlTestSupport::containsText(
            html,
            R"(name="username")"
        )
    );
}

TEST(PortalApplicationTests, ProjectTypesPageDisplaysProjectCounts) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectTypesPage>();

    for (const auto& type :
         app.dataset().projectTypes()) {
        const auto count =
            std::count_if(
                app.dataset().projects().begin(),
                app.dataset().projects().end(),
                [&type](const PortalProject& project) {
                    return project.projectTypeId ==
                           type.id;
                }
            );

        EXPECT_TRUE(
            HtmlTestSupport::containsText(
                html,
                type.title
            )
        );

        EXPECT_TRUE(
            HtmlTestSupport::containsText(
                html,
                std::to_string(count)
            )
        );
         }
}

TEST(PortalApplicationTests, ProjectTypesPageHidesDeleteForUsedType) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto usedId =
        app.dataset()
            .projects()
            .front()
            .projectTypeId;

    const auto html =
        app.render<PortalProjectTypesPage>();

    const auto deleteAction =
        "/project-types/" +
        std::to_string(usedId) +
        "/delete";

    EXPECT_FALSE(
        HtmlTestSupport::containsText(
            html,
            deleteAction
        )
    );
}

TEST(PortalApplicationTests, ProjectTypesPageShowsDeleteForUnusedType) {
    auto dataset =
        DemoDataset::create();

    dataset.addProjectType({
        .id = 99,
        .code = "unused",
        .title = "Unused"
    });

    PortalApplicationTestHost app(
        std::move(dataset)
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectTypesPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "/project-types/99/delete"
        )
    );
}

TEST(PortalApplicationTests, RegularUserCannotManageProjectTypes) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsUser();

    const auto html =
        app.render<PortalProjectTypesPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "Access denied"
        )
    );

    EXPECT_FALSE(
        HtmlTestSupport::containsText(
            html,
            R"(action="/project-types/)"
        )
    );
}

TEST(PortalApplicationTests, AdminCreatesProjectType) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result =
        app.execute<PortalCreateProjectTypeAction>({
            {"code", "support"},
            {"title", "Support"}
        });

    EXPECT_EQ(
        result.location(),
        "/project-types?success=project_type_created"
    );

    const auto& created =
        app.dataset().projectTypes().back();

    EXPECT_EQ(
        created.code,
        "support"
    );

    EXPECT_EQ(
        created.title,
        "Support"
    );

    EXPECT_GT(
        created.id,
        0
    );
}

TEST(PortalApplicationTests, ProjectTypeCreateFormUsesSchemaMetadata) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectTypesPage>();

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="code")",
            "required"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="title")",
            "required"
        )
    );
}

TEST(PortalApplicationTests, AdminRendersProjectTypeEditForm) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto type =
        app.dataset().projectTypes().front();

    const auto html =
        app.render<PortalProjectTypeEditPage>(
            {},
            {
                {
                    "id",
                    std::to_string(type.id)
                }
            }
        );

    const auto code =
        HtmlTestSupport::attributeValue(
            html,
            R"(name="code")",
            "value"
        );

    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(*code, type.code);

    const auto title =
        HtmlTestSupport::attributeValue(
            html,
            R"(name="title")",
            "value"
        );

    ASSERT_TRUE(title.has_value());
    EXPECT_EQ(*title, type.title);
}

TEST(PortalApplicationTests, UpdatesOnlyProvidedProjectTypeFields) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto before =
        app.dataset().projectTypes().front();

    const auto result =
        app.execute<PortalUpdateProjectTypeAction>(
            {
                {"title", "Updated Type"}
            },
            {
                {
                    "id",
                    std::to_string(before.id)
                }
            }
        );

    EXPECT_EQ(
        result.location(),
        "/project-types?success=project_type_updated"
    );

    const auto& after =
        app.dataset().projectTypes().front();

    EXPECT_EQ(
        after.title,
        "Updated Type"
    );

    EXPECT_EQ(
        after.code,
        before.code
    );
}

TEST(PortalApplicationTests, DeletesUnusedProjectType) {
    auto dataset =
        DemoDataset::create();

    dataset.addProjectType({
        .id = 99,
        .code = "unused",
        .title = "Unused"
    });

    PortalApplicationTestHost app(
        std::move(dataset)
    );

    app.loginAsAdmin();

    const auto result =
        app.execute<PortalDeleteProjectTypeAction>(
            {},
            {
                {"id", "99"}
            }
        );

    EXPECT_EQ(
        result.location(),
        "/project-types?success=project_type_deleted"
    );

    EXPECT_TRUE(
        std::none_of(
            app.dataset().projectTypes().begin(),
            app.dataset().projectTypes().end(),
            [](const PortalProjectType& type) {
                return type.id == 99;
            }
        )
    );
}

TEST(PortalApplicationTests, RejectsDeletingUsedProjectType) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto usedId =
        app.dataset()
            .projects()
            .front()
            .projectTypeId;

    const auto result =
        app.execute<PortalDeleteProjectTypeAction>(
            {},
            {
                {
                    "id",
                    std::to_string(usedId)
                }
            }
        );

    EXPECT_EQ(
        result.location(),
        "/project-types?error=project_type_in_use"
    );

    EXPECT_TRUE(
        std::any_of(
            app.dataset().projectTypes().begin(),
            app.dataset().projectTypes().end(),
            [usedId](
                const PortalProjectType& projectType
            ) {
                return projectType.id == usedId;
            }
        )
    );
}