#include "portal_application_test_host.hpp"
#include "../support/html_test_support.hpp"

#include "../../examples/portal_demo/actions/create_project_action.hpp"
#include "../../examples/portal_demo/data/demo_dataset.hpp"
#include "../../examples/portal_demo/actions/create_user_action.hpp"
#include "../../examples/portal_demo/actions/update_project_action.hpp"
#include "../../examples/portal_demo/actions/delete_project_action.hpp"
#include "../../examples/portal_demo/pages/projects_page.hpp"
#include "../../examples/portal_demo/pages/project_edit_page.hpp"
#include "../../examples/portal_demo/pages/users_page.hpp"

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

    EXPECT_EQ(
        app.dataset().projects().back().title,
        "Application Test Project"
    );

    EXPECT_EQ(
        app.dataset().projects().back().projectTypeId,
        2
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
            {"username", "john"},
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

    EXPECT_EQ(
        app.dataset().users().back().username,
        "john"
    );
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

TEST(PortalApplicationTests, AdminUpdatesProject) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    app.loginAsAdmin();

    const auto result =
        app.post<PortalUpdateProjectAction>(
            {
                {"title", "Updated Portal"},
                {"projectTypeId", "2"},
                {"status", "done"}
            },
            {
                {"id", "1"}
            }
        );

    EXPECT_EQ(
        result.location(),
        "/projects/1?success=project_updated"
    );

    EXPECT_EQ(
        app.dataset().projects()[0].title,
        "Updated Portal"
    );

    EXPECT_EQ(
        app.dataset().projects()[0].projectTypeId,
        2
    );

    EXPECT_EQ(
        app.dataset().projects()[0].status,
        "done"
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