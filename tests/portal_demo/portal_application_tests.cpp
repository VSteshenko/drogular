#include "portal_application_test_host.hpp"
#include "support/html_test_support.hpp"

#include "features/projects/actions/create_project_action.hpp"
#include "data/demo_dataset.hpp"
#include "features/users/actions/create_user_action.hpp"
#include "features/users/actions/update_user_action.hpp"
#include "features/projects/actions/update_project_action.hpp"
#include "features/projects/actions/delete_project_action.hpp"
#include "features/projects/pages/projects_page.hpp"
#include "features/projects/pages/project_details_page.hpp"
#include "features/projects/pages/project_edit_page.hpp"
#include "features/users/pages/users_page.hpp"
#include "features/users/pages/user_edit_page.hpp"
#include "features/roles/pages/roles_page.hpp"
#include "features/roles/pages/role_edit_page.hpp"
#include "features/roles/actions/create_role_action.hpp"
#include "features/roles/actions/update_role_action.hpp"
#include "features/roles/actions/delete_role_action.hpp"
#include "features/project_types/pages/project_types_page.hpp"
#include "features/project_types/pages/project_type_edit_page.hpp"
#include "features/project_types/actions/create_project_type_action.hpp"
#include "features/project_types/actions/update_project_type_action.hpp"
#include "features/project_types/actions/delete_project_type_action.hpp"
#include "features/departments/pages/departments_page.hpp"
#include "features/departments/pages/department_details_page.hpp"
#include "features/departments/pages/department_edit_page.hpp"
#include "features/dashboard/pages/dashboard_page.hpp"
#include "features/departments/actions/create_department_action.hpp"
#include "features/departments/actions/update_department_action.hpp"
#include "features/department_members/actions/add_department_member_action.hpp"
#include "features/department_members/actions/remove_department_member_action.hpp"

#include <algorithm>

#include <gtest/gtest.h>

TEST(PortalApplicationTests, DashboardUsesNestedLoopMetadataForQuickLinks) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalDashboardPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(data-section-number="1")"
        )
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(data-loop-depth="0")"
        )
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(data-link-number="1.1")"
        )
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(data-loop-depth="1")"
        )
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(href="/project-types")"
        )
    );
}

TEST(PortalApplicationTests, DashboardFiltersAdminQuickLinksForRegularUser) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsUser();

    const auto html = app.render<PortalDashboardPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(href="/projects")"
        )
    );
    EXPECT_FALSE(
        HtmlTestSupport::containsText(
            html,
            R"(href="/admin")"
        )
    );
    EXPECT_FALSE(
        HtmlTestSupport::containsText(
            html,
            R"(href="/roles")"
        )
    );
}

TEST(PortalApplicationTests, ProjectsPageUsesForeachEmptyState) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalProjectsPage>({
        {"search", "no-project-can-match-this"}
    });

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "No projects found."
        )
    );
}

TEST(PortalApplicationTests, UsersPageUsesForeachEmptyState) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalUsersPage>({
        {"search", "no-user-can-match-this"}
    });

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "No users found."
        )
    );
}

TEST(PortalApplicationTests, DepartmentsPageUsesForeachEmptyState) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalDepartmentsPage>({
        {"search", "no-department-can-match-this"}
    });

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "No departments found."
        )
    );
}

TEST(PortalApplicationTests, DepartmentDetailsUsesForeachEmptyForMembers) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalDepartmentDetailsPage>(
        {},
        {{"id", "3"}}
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "This department has no members."
        )
    );
}

TEST(PortalApplicationTests, DashboardTranslatesQuickLinksThroughExpressionFunction) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalDashboardPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "Workspace"
        )
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "Administration"
        )
    );
    EXPECT_FALSE(
        HtmlTestSupport::containsText(
            html,
            "dashboard.section.workspace"
        )
    );
}

TEST(PortalApplicationTests, ProjectsPageFormatsStatusWithSwitch) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalProjectsPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "Active"
        )
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "Paused"
        )
    );
}

TEST(PortalApplicationTests, DepartmentDetailsUsesCollectionFunctionForCandidates) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalDepartmentDetailsPage>(
        {},
        {{"id", "1"}}
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "Add member"
        )
    );
}

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
        21
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
        20
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
        9
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
        8
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
        8
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
        19
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

    EXPECT_TRUE(
        HtmlTestSupport::optionSelectedInSelect(
            html,
            R"(name="projectTypeId")",
            "1"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsOption(
            HtmlTestSupport::elementContent(
                html,
                R"(name="projectTypeId")",
                "</select>"
            ),
            "2"
        )
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
        HtmlTestSupport::optionSelectedInSelect(
            html,
            R"(name="role")",
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

TEST(PortalApplicationTests, ProjectTypesPageBuildsReferenceListUrls) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto projectType =
        app.dataset()
            .projectTypes()
            .front();

    const auto html =
        app.render<PortalProjectTypesPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "/project-types/" +
            std::to_string(projectType.id) +
            "/edit"
        )
    );
}

TEST(PortalApplicationTests, RolesPageDisplaysUserCounts) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalRolesPage>();

    for (const auto& role :
         app.dataset().roles()) {
        const auto count =
            std::count_if(
                app.dataset().users().begin(),
                app.dataset().users().end(),
                [&role](const PortalUser& user) {
                    return user.role ==
                           role.code;
                }
            );

        EXPECT_TRUE(
            HtmlTestSupport::containsText(
                html,
                role.title
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

TEST(PortalApplicationTests, RolesPageHidesDeleteForUsedRole) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto usedCode =
        app.dataset().users().front().role;

    const auto role =
        std::find_if(
            app.dataset().roles().begin(),
            app.dataset().roles().end(),
            [&usedCode](const PortalRole& item) {
                return item.code == usedCode;
            }
        );

    ASSERT_NE(
        role,
        app.dataset().roles().end()
    );

    const auto html =
        app.render<PortalRolesPage>();

    EXPECT_FALSE(
        HtmlTestSupport::containsText(
            html,
            "/roles/" +
            std::to_string(role->id) +
            "/delete"
        )
    );
}

TEST(PortalApplicationTests, RolesPageShowsDeleteForUnusedRole) {
    auto dataset =
        DemoDataset::create();

    dataset.addRole({
        .id = 99,
        .code = "unused",
        .title = "Unused"
    });

    PortalApplicationTestHost app(
        std::move(dataset)
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalRolesPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "/roles/99/delete"
        )
    );
}

TEST(PortalApplicationTests, AdminCreatesRole) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result =
        app.execute<PortalCreateRoleAction>({
            {"code", "manager"},
            {"title", "Manager"}
        });

    EXPECT_EQ(
        result.location(),
        "/roles?success=role_created"
    );

    const auto& role =
        app.dataset().roles().back();

    EXPECT_EQ(role.code, "manager");
    EXPECT_EQ(role.title, "Manager");
}

TEST(PortalApplicationTests, AdminRendersRoleEditForm) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto role =
        app.dataset().roles().front();

    const auto html =
        app.render<PortalRoleEditPage>(
            {},
            {
                {
                    "id",
                    std::to_string(role.id)
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
    EXPECT_EQ(*code, role.code);

    const auto title =
        HtmlTestSupport::attributeValue(
            html,
            R"(name="title")",
            "value"
        );

    ASSERT_TRUE(title.has_value());
    EXPECT_EQ(*title, role.title);

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

TEST(PortalApplicationTests, UpdatesOnlyProvidedRoleFields) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto before =
        app.dataset().roles().front();

    const auto result =
        app.execute<PortalUpdateRoleAction>(
            {
                {"title", "Updated role"}
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
        "/roles?success=role_updated"
    );

    const auto& after =
        app.dataset().roles().front();

    EXPECT_EQ(
        after.code,
        before.code
    );

    EXPECT_EQ(
        after.title,
        "Updated role"
    );
}

TEST(PortalApplicationTests, AdminDeletesUnusedRole) {
    auto dataset =
        DemoDataset::create();

    dataset.addRole({
        .id = 99,
        .code = "unused",
        .title = "Unused"
    });

    PortalApplicationTestHost app(
        std::move(dataset)
    );

    app.loginAsAdmin();

    const auto result =
        app.execute<PortalDeleteRoleAction>(
            {},
            {
                {"id", "99"}
            }
        );

    EXPECT_EQ(
        result.location(),
        "/roles?success=role_deleted"
    );

    EXPECT_TRUE(
        std::none_of(
            app.dataset().roles().begin(),
            app.dataset().roles().end(),
            [](const PortalRole& role) {
                return role.id == 99;
            }
        )
    );
}

TEST(PortalApplicationTests, RejectsDeletingUsedRole) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto usedCode =
        app.dataset().users().front().role;

    const auto role =
        std::find_if(
            app.dataset().roles().begin(),
            app.dataset().roles().end(),
            [&usedCode](const PortalRole& item) {
                return item.code == usedCode;
            }
        );

    ASSERT_NE(
        role,
        app.dataset().roles().end()
    );

    const auto result =
        app.execute<PortalDeleteRoleAction>(
            {},
            {
                {
                    "id",
                    std::to_string(role->id)
                }
            }
        );

    EXPECT_EQ(
        result.location(),
        "/roles?error=role_in_use"
    );

    EXPECT_TRUE(
        std::any_of(
            app.dataset().roles().begin(),
            app.dataset().roles().end(),
            [id = role->id](const PortalRole& item) {
                return item.id == id;
            }
        )
    );
}

TEST(PortalApplicationTests, ProjectsPageFiltersProjectsBySearch) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto expected =
        std::find_if(
            app.dataset().projects().begin(),
            app.dataset().projects().end(),
            [](const PortalProject& project) {
                return project.title ==
                    "Inventory Manager";
            }
        );

    ASSERT_NE(
        expected,
        app.dataset().projects().end()
    );

    const auto html =
        app.render<PortalProjectsPage>({
            {
                "search",
                expected->title
            }
        });

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            expected->title
        )
    );

    for (const auto& project :
         app.dataset().projects()) {
        if (project.id == expected->id) {
            continue;
        }

        EXPECT_FALSE(
            HtmlTestSupport::containsText(
                html,
                project.title
            )
        );
    }
}

TEST(PortalApplicationTests, ProjectDetailsPreservesProjectsSearchReturnUrl) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto project =
        app.dataset().projects().front();

    const auto html =
        app.render<PortalProjectDetailsPage>(
            {
                {
                    "returnUrl",
                    "/projects?search=portal"
                }
            },
            {
                {
                    "id",
                    std::to_string(project.id)
                }
            }
        );

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(href="/projects?search=portal")"
        )
    );
}

TEST(PortalApplicationTests, ProjectsPagePreservesSelectedStatusFilter) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectsPage>({
            {"status", "active"}
        });

    EXPECT_TRUE(
        HtmlTestSupport::optionSelectedInSelect(
            html,
            R"(id="statusFilter")",
            "active"
        )
    );
}

TEST(PortalApplicationTests, ProjectDetailsPreservesSearchAndStatusReturnUrl) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto project =
        app.dataset().projects().front();

    const auto html =
        app.render<PortalProjectDetailsPage>(
            {
                {
                    "returnUrl",
                    "/projects?search=port&status=active"
                }
            },
            {
                {
                    "id",
                    std::to_string(project.id)
                }
            }
        );

    const auto href =
        HtmlTestSupport::attributeValue(
            html,
            R"(id="projectsBackFromDetailsLink")",
            "href"
        );

    ASSERT_TRUE(href.has_value());

    EXPECT_EQ(
        HtmlTestSupport::decodeEntities(*href),
        "/projects?search=port&status=active"
    );
}

TEST(PortalApplicationTests, ProjectsPagePreservesSelectedProjectTypeFilter) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto projectType =
        app.dataset()
            .projectTypes()
            .front();

    const auto html =
        app.render<PortalProjectsPage>({
            {
                "projectTypeId",
                std::to_string(
                    projectType.id
                )
            }
        });

    EXPECT_TRUE(
        HtmlTestSupport::
            optionSelectedInSelect(
                html,
                R"(id="projectTypeFilter")",
                std::to_string(
                    projectType.id
                )
            )
    );
}

TEST(PortalApplicationTests, ProjectDetailsPreservesAllProjectFilters) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto project =
        app.dataset().projects().front();

    const auto returnUrl =
        "/projects?search=port"
        "&status=active"
        "&projectTypeId=1"
        "&ownerId=2";

    const auto html =
        app.render<PortalProjectDetailsPage>(
            {
                {
                    "returnUrl",
                    returnUrl
                }
            },
            {
                {
                    "id",
                    std::to_string(
                        project.id
                    )
                }
            }
        );

    const auto href =
        HtmlTestSupport::attributeValue(
            html,
            R"(id="projectsBackFromDetailsLink")",
            "href"
        );

    ASSERT_TRUE(href.has_value());

    EXPECT_EQ(
        HtmlTestSupport::decodeEntities(
            *href
        ),
        returnUrl
    );
}

TEST(PortalApplicationTests, ProjectsPagePreservesSelectedOwnerFilter) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto ownerId =
        app.dataset()
            .projects()
            .front()
            .ownerId;

    const auto html =
        app.render<PortalProjectsPage>({
            {
                "ownerId",
                std::to_string(ownerId)
            }
        });

    EXPECT_TRUE(
        HtmlTestSupport::
            optionSelectedInSelect(
                html,
                R"(id="projectOwnerFilter")",
                std::to_string(ownerId)
            )
    );
}

TEST(PortalApplicationTests, ProjectsPagePreservesSelectedSorting) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectsPage>({
            {"sort", "status"},
            {"direction", "desc"}
        });

    EXPECT_TRUE(
        HtmlTestSupport::
            optionSelectedInSelect(
                html,
                R"(id="projectSort")",
                "status"
            )
    );

    EXPECT_TRUE(
        HtmlTestSupport::
            optionSelectedInSelect(
                html,
                R"(id="projectSortDirection")",
                "desc"
            )
    );
}

TEST(PortalApplicationTests, ProjectsPageSelectsDefaultSorting) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectsPage>();

    EXPECT_TRUE(
        HtmlTestSupport::
            optionSelectedInSelect(
                html,
                R"(id="projectSort")",
                "title"
            )
    );

    EXPECT_TRUE(
        HtmlTestSupport::
            optionSelectedInSelect(
                html,
                R"(id="projectSortDirection")",
                "asc"
            )
    );
}
TEST(PortalApplicationTests, ProjectsPageShowsPagination) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectsPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(aria-label="Project pages")"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(href="/projects?page=2")"
        )
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(aria-current="page")"
        )
    );
}

TEST(PortalApplicationTests, ProjectsPageShowsSecondPage) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    auto projects =
        app.dataset().projects();

    std::stable_sort(
        projects.begin(),
        projects.end(),
        [](const PortalProject& left, const PortalProject& right) {
            if (left.title != right.title) {
                return left.title < right.title;
            }
            return left.id < right.id;
        }
    );

    const auto html =
        app.render<PortalProjectsPage>(
            {{"page", "2"}}
        );

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "data-project-id=\"" + std::to_string(projects[10].id) + "\""
        )
    );

    EXPECT_FALSE(
        HtmlTestSupport::containsText(
            html,
            "data-project-id=\"" + std::to_string(projects[0].id) + "\""
        )
    );
}

TEST(PortalApplicationTests, ProjectPaginationPreservesFiltersAndSorting) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html =
        app.render<PortalProjectsPage>({
            {"search", "a"},
            {"sort", "id"},
            {"direction", "desc"}
        });

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(href="/projects?search=a&amp;sort=id&amp;direction=desc&amp;page=2")"
        )
    );
}

TEST(PortalApplicationTests, AdminCreatesDepartment) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result = app.post<PortalCreateDepartmentAction>({
        {"name", "Quality Assurance"},
        {"description", "Product quality and testing"},
        {"managerId", "3"},
        {"isActive", "on"}
    });

    EXPECT_EQ(
        result.location(),
        "/departments?success=created"
    );
    ASSERT_EQ(app.departmentCount(), 4);

    const auto& department =
        app.dataset().departments().back();

    EXPECT_EQ(department.name, "Quality Assurance");
    EXPECT_EQ(department.description, "Product quality and testing");
    EXPECT_EQ(department.managerId, 3);
    EXPECT_TRUE(department.isActive);
}

TEST(PortalApplicationTests, UserCannotCreateDepartment) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsUser();

    const auto result = app.post<PortalCreateDepartmentAction>({
        {"name", "Quality Assurance"},
        {"managerId", "3"}
    });

    EXPECT_EQ(
        result.location(),
        "/departments?error=access_denied"
    );
    EXPECT_EQ(app.departmentCount(), 3);
}

TEST(PortalApplicationTests, AdminUpdatesDepartment) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result = app.execute<PortalUpdateDepartmentAction>(
        {
            {"name", "Platform Engineering"},
            {"description", "Platform ownership"},
            {"managerId", "4"}
        },
        {{"id", "1"}}
    );

    EXPECT_EQ(
        result.location(),
        "/departments?success=updated"
    );

    const auto& department =
        app.dataset().departments().front();

    EXPECT_EQ(department.name, "Platform Engineering");
    EXPECT_EQ(department.description, "Platform ownership");
    EXPECT_EQ(department.managerId, 4);
    EXPECT_FALSE(department.isActive);
}

TEST(PortalApplicationTests, DepartmentsPageRendersDepartmentData) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalDepartmentsPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(html, "Engineering")
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(html, "Product and platform engineering")
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(html, R"(href="/departments/1")")
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(html, R"(href="/departments/1/edit")")
    );
}

TEST(PortalApplicationTests, DepartmentDetailsShowsMembersAndCandidates) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalDepartmentDetailsPage>(
        {},
        {{"id", "1"}}
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsText(html, "Engineering")
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(html, "admin")
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(html, "user")
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(action="/departments/1/members/1/remove")"
        )
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(action="/departments/1/members/add")"
        )
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            R"(<option value="3">)"
        )
    );
}

TEST(PortalApplicationTests, DepartmentEditPageSelectsCurrentManager) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto html = app.render<PortalDepartmentEditPage>(
        {},
        {{"id", "2"}}
    );

    EXPECT_TRUE(
        HtmlTestSupport::containsText(html, "Operations")
    );
    EXPECT_TRUE(
        HtmlTestSupport::optionSelected(
            html,
            "2"
        )
    );
}

TEST(PortalApplicationTests, AdminAddsDepartmentMember) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result = app.execute<PortalAddDepartmentMemberAction>(
        {{"userId", "3"}},
        {{"id", "1"}}
    );

    EXPECT_EQ(
        result.location(),
        "/departments/1?success=member_added"
    );
    EXPECT_EQ(app.departmentMemberCount(), 5);

    const auto& member =
        app.dataset().departmentMembers().back();

    EXPECT_EQ(member.departmentId, 1);
    EXPECT_EQ(member.userId, 3);
}

TEST(PortalApplicationTests, DuplicateDepartmentMemberIsRejected) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result = app.execute<PortalAddDepartmentMemberAction>(
        {{"userId", "1"}},
        {{"id", "1"}}
    );

    EXPECT_EQ(
        result.location(),
        "/departments/1?error=duplicate_member"
    );
    EXPECT_EQ(app.departmentMemberCount(), 4);
}

TEST(PortalApplicationTests, UserCannotAddDepartmentMember) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsUser();

    const auto result = app.execute<PortalAddDepartmentMemberAction>(
        {{"userId", "3"}},
        {{"id", "1"}}
    );

    EXPECT_EQ(
        result.location(),
        "/departments/1?error=access_denied"
    );
    EXPECT_EQ(app.departmentMemberCount(), 4);
}

TEST(PortalApplicationTests, AdminRemovesDepartmentMember) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result = app.execute<PortalRemoveDepartmentMemberAction>(
        {},
        {
            {"id", "1"},
            {"userId", "2"}
        }
    );

    EXPECT_EQ(
        result.location(),
        "/departments/1?success=member_removed"
    );
    EXPECT_EQ(app.departmentMemberCount(), 3);

    const auto removed = std::none_of(
        app.dataset().departmentMembers().begin(),
        app.dataset().departmentMembers().end(),
        [](const PortalDepartmentMember& member) {
            return member.departmentId == 1 && member.userId == 2;
        }
    );
    EXPECT_TRUE(removed);
}

TEST(PortalApplicationTests, MissingDepartmentMemberCannotBeRemoved) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result = app.execute<PortalRemoveDepartmentMemberAction>(
        {},
        {
            {"id", "1"},
            {"userId", "8"}
        }
    );

    EXPECT_EQ(
        result.location(),
        "/departments/1?error=member_not_found"
    );
    EXPECT_EQ(app.departmentMemberCount(), 4);
}