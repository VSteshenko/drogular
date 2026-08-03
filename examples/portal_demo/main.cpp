#include "startup/portal_demo_startup.hpp"
#include "startup/portal_graphql_server_factory.hpp"
#include "features/localization/actions/language_action.hpp"
#include "features/diagnostics/portal_inspection_contributor.hpp"
#include "features/dashboard/pages/dashboard_page.hpp"
#include "features/auth/pages/login_page.hpp"
#include "features/auth/actions/login_action.hpp"
#include "features/auth/actions/logout_action.hpp"
#include "features/users/pages/users_page.hpp"
#include "features/users/pages/user_edit_page.hpp"
#include "features/roles/pages/role_edit_page.hpp"
#include "features/roles/actions/create_role_action.hpp"
#include "features/roles/actions/update_role_action.hpp"
#include "features/roles/actions/delete_role_action.hpp"
#include "features/roles/pages/roles_page.hpp"
#include "features/admin/pages/admin_page.hpp"
#include "features/project_types/pages/project_types_page.hpp"
#include "features/project_types/actions/create_project_type_action.hpp"
#include "features/project_types/actions/delete_project_type_action.hpp"
#include "features/project_types/actions/update_project_type_action.hpp"
#include "features/project_types/pages/project_type_edit_page.hpp"
#include "features/users/actions/create_user_action.hpp"
#include "features/users/actions/update_user_action.hpp"
#include "features/offline/pages/offline_page.hpp"
#include "features/localization/support/portal_translations.hpp"
#include "features/projects/actions/create_project_action.hpp"
#include "features/projects/pages/projects_page.hpp"
#include "features/projects/pages/project_details_page.hpp"
#include "features/projects/pages/project_edit_page.hpp"
#include "features/projects/actions/update_project_action.hpp"
#include "features/projects/actions/delete_project_action.hpp"
#include "features/projects/graphql/portal_graphql_project_provider.hpp"
#include <drogular/in_process_graphql_client.hpp>
#include "providers/graphql/portal_dataset_graphql_adapter.hpp"
#include "features/users/graphql/portal_graphql_user_provider.hpp"
#include "features/roles/graphql/portal_graphql_role_provider.hpp"
#include "features/project_types/graphql/portal_graphql_project_type_provider.hpp"
#include "data/demo_dataset.hpp"
#include "features/departments/graphql/portal_graphql_department_provider.hpp"
#include "features/departments/pages/departments_page.hpp"
#include "features/departments/pages/department_edit_page.hpp"
#include "features/departments/pages/department_details_page.hpp"
#include "features/department_members/graphql/portal_graphql_department_member_provider.hpp"
#include "features/department_members/actions/add_department_member_action.hpp"
#include "features/department_members/actions/remove_department_member_action.hpp"
#include "features/departments/actions/create_department_action.hpp"
#include "features/departments/actions/update_department_action.hpp"
#include "ui/components/portal_select_component.hpp"
#include "ui/components/portal_pagination_component.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>
#include <drogular/session_store.hpp>

#include <exception>
#include <iostream>

int main(
    int argc,
    char* argv[]
) {
    PortalDemoOptions startupOptions;

    try {
        startupOptions =
            PortalDemoStartup::parse(
                argc,
                argv
            );
    } catch (const std::exception& error) {
        std::cerr
            << "Error: "
            << error.what()
            << "\n\n";

        PortalDemoStartup::printHelp(
            std::cerr
        );

        return 1;
    }

    if (startupOptions.showHelp) {
        PortalDemoStartup::printHelp();
        return 0;
    }

    drogular::App app;

    app.templateRoot(
        "examples/portal_demo/templates"
    )
    .staticFiles(
       "/assets",
       "examples/portal_demo/public"
    )
    .templateCache(false)
    .staticFileCacheProfile(
        drogular::StaticFileCacheProfile::Development
    )
    .serviceWorker(
        "examples/portal_demo/public/service-worker.js"
    )
    .offlinePage<PortalOfflinePage>()
    .enableDiagnosticsPage();

    app.services().addFactory<drogular::TranslationProvider>(
        drogular::ServiceLifetime::Singleton,
        [] {
            return std::make_shared<PortalTranslations>();
        }
    );

    app.services().add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto dataset =
        std::make_shared<PortalDataset>(
            DemoDataset::create()
        );

    const auto inspectionContributors =
        app.services().service<drogular::InspectionContributors>();

    if (inspectionContributors != nullptr) {
        inspectionContributors->add(
            std::make_shared<PortalInspectionContributor>(dataset)
        );
    }

    auto graphQLServer =
        createPortalGraphQLServer(dataset);

    auto graphQLClient =
        std::make_shared<drogular::InProcessGraphQLClient>(
            graphQLServer
        );

    auto userProvider =
        std::make_shared<PortalGraphQLUserProvider>(
            graphQLClient
        );

    app.services().addFactory<PortalRoleProvider>(
        drogular::ServiceLifetime::Singleton,
        [graphQLClient] {
            return std::make_shared<PortalGraphQLRoleProvider>(
                graphQLClient
            );
        }
    );

    app.services().addFactory<PortalUserProvider>(
        drogular::ServiceLifetime::Singleton,
        [userProvider] {
            return userProvider;
        }
    );

    app.services().addFactory<PortalProjectTypeProvider>(
        drogular::ServiceLifetime::Singleton,
        [graphQLClient] {
            return std::make_shared<PortalGraphQLProjectTypeProvider>(
                graphQLClient
            );
        }
    );

    app.services().addFactory<PortalDepartmentProvider>(
        drogular::ServiceLifetime::Singleton,
        [graphQLClient] {
            return std::make_shared<PortalGraphQLDepartmentProvider>(
                graphQLClient
            );
        }
    );

    app.services().addFactory<PortalDepartmentMemberProvider>(
        drogular::ServiceLifetime::Singleton,
        [graphQLClient] {
            return std::make_shared<PortalGraphQLDepartmentMemberProvider>(
                graphQLClient
            );
        }
    );

    app.services().addFactory<PortalProjectProvider>(
        drogular::ServiceLifetime::Singleton,
        [graphQLClient, userProvider] {
            return std::make_shared<PortalGraphQLProjectProvider>(
                graphQLClient
            );
        }
    );

    app.component<PortalSelectComponent>();
    app.component<PortalPaginationComponent>();

    app.page<PortalLoginPage>("/");
    app.page<PortalLoginPage>("/login");
    app.page<PortalDashboardPage>("/dashboard");
    app.page<PortalUsersPage>("/users");
    app.page<PortalUserEditPage>("/users/{id}/edit");
    app.page<PortalRolesPage>("/roles");
    app.page<PortalRoleEditPage>("/roles/{id}/edit");
    app.action<PortalUpdateRoleAction>("/roles/{id}/update");
    app.action<PortalDeleteRoleAction>("/roles/{id}/delete");
    app.page<PortalAdminPage>("/admin");
    app.page<PortalProjectTypesPage>("/project-types");
    app.page<PortalProjectTypeEditPage>("/project-types/{id}/edit");
    app.action<PortalUpdateProjectTypeAction>("/project-types/{id}/update");
    app.action<PortalDeleteProjectTypeAction>("/project-types/{id}/delete");
    app.page<PortalDepartmentsPage>("/departments");
    app.page<PortalDepartmentDetailsPage>("/departments/{id}");
    app.page<PortalDepartmentEditPage>("/departments/{id}/edit");
    app.page<PortalProjectsPage>("/projects");
    app.page<PortalProjectDetailsPage>("/projects/{id}");
    app.page<PortalProjectEditPage>("/projects/{id}/edit");

    app.action<PortalLanguageAction>("/language");
    app.action<PortalLoginAction>("/login");
    app.action<PortalLogoutAction>("/logout");
    app.action<PortalCreateUserAction>("/users/create");
    app.action<PortalUpdateUserAction>("/users/{id}/update");
    app.action<PortalCreateRoleAction>("/roles/create");
    app.action<PortalCreateDepartmentAction>("/departments/create");
    app.action<PortalUpdateDepartmentAction>("/departments/{id}/update");
    app.action<PortalAddDepartmentMemberAction>("/departments/{id}/members/add");
    app.action<PortalRemoveDepartmentMemberAction>("/departments/{id}/members/{userId}/remove");
    app.action<PortalCreateProjectAction>("/projects/create");
    app.action<PortalUpdateProjectAction>("/projects/{id}/update");
    app.action<PortalDeleteProjectAction>("/projects/{id}/delete");
    app.action<PortalCreateProjectTypeAction>("/project-types/create");

    PortalDemoStartup::printBanner(
        startupOptions
    );

    app.run(
        startupOptions.port
    );

    return 0;
}