#include "actions/language_action.hpp"
#include "pages/dashboard_page.hpp"
#include "pages/login_page.hpp"
#include "actions/login_action.hpp"
#include "actions/logout_action.hpp"
#include "pages/users_page.hpp"
#include "pages/admin_page.hpp"
#include "providers/memory/portal_memory_user_provider.hpp"
#include "actions/create_user_action.hpp"
#include "pages/offline_page.hpp"
#include "localization/portal_translations.hpp"
#include "actions/create_project_action.hpp"
#include "pages/projects_page.hpp"
#include "pages/project_details_page.hpp"
#include "pages/project_edit_page.hpp"
#include "actions/update_project_action.hpp"
#include "actions/delete_project_action.hpp"
#include "providers/graphql/portal_graphql_project_provider.hpp"
#include "providers/graphql/portal_dataset_graphql_client.hpp"
#include "providers/graphql/portal_dataset_graphql_adapter.hpp"
#include "providers/graphql/portal_graphql_user_provider.hpp"
#include "data/demo_dataset.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>
#include <drogular/session_store.hpp>

int main() {
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
    .offlinePage<PortalOfflinePage>();

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

    auto graphQLClient =
        std::make_shared<PortalDatasetGraphQLClient>(
            dataset
        );

    auto userProvider =
        std::make_shared<PortalGraphQLUserProvider>(
            graphQLClient
        );

    app.services().addFactory<PortalUserProvider>(
        drogular::ServiceLifetime::Singleton,
        [userProvider] {
            return userProvider;
        }
    );

    app.services().addFactory<PortalProjectProvider>(
        drogular::ServiceLifetime::Singleton,
        [graphQLClient, userProvider] {
            return std::make_shared<PortalGraphQLProjectProvider>(
                graphQLClient,
                userProvider
            );
        }
    );

    app.page<PortalLoginPage>("/");
    app.page<PortalLoginPage>("/login");
    app.page<PortalDashboardPage>("/dashboard");
    app.page<PortalUsersPage>("/users");
    app.page<PortalAdminPage>("/admin");
    app.page<PortalProjectsPage>("/projects");
    app.page<PortalProjectDetailsPage>("/projects/{id}");
    app.page<PortalProjectEditPage>("/projects/{id}/edit");

    app.action<PortalLanguageAction>("/language");
    app.action<PortalLoginAction>("/login");
    app.action<PortalLogoutAction>("/logout");
    app.action<PortalCreateUserAction>("/users/create");
    app.action<PortalCreateProjectAction>("/projects/create");
    app.action<PortalUpdateProjectAction>("/projects/{id}/update");
    app.action<PortalDeleteProjectAction>("/projects/{id}/delete");

    app.run(8083);

    return 0;
}