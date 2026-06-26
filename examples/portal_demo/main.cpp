#include "actions/language_action.hpp"
#include "pages/dashboard_page.hpp"
#include "pages/login_page.hpp"
#include "actions/login_action.hpp"
#include "actions/logout_action.hpp"
#include "pages/users_page.hpp"
#include "pages/admin_page.hpp"
#include "portal_user_repository.hpp"
#include "actions/create_user_action.hpp"
#include "pages/offline_page.hpp"
#include "localization/portal_translations.hpp"
#include "actions/create_project_action.hpp"
#include "pages/projects_page.hpp"
#include "portal_project_repository.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>
#include "drogular/session_store.hpp"

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

    app.services().add<PortalTranslations>(
        drogular::ServiceLifetime::Singleton
    );

    app.services().add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    app.services().add<PortalUserRepository>(
        drogular::ServiceLifetime::Singleton
    );

    app.services().add<PortalProjectRepository>(
        drogular::ServiceLifetime::Singleton
    );

    app.page<PortalLoginPage>("/");
    app.page<PortalLoginPage>("/login");
    app.page<PortalDashboardPage>("/dashboard");
    app.page<PortalUsersPage>("/users");
    app.page<PortalAdminPage>("/admin");
    app.page<PortalProjectsPage>("/projects");

    app.action<PortalLanguageAction>("/language");
    app.action<PortalLoginAction>("/login");
    app.action<PortalLogoutAction>("/logout");
    app.action<PortalCreateUserAction>("/users/create");
    app.action<PortalCreateProjectAction>("/projects/create");

    app.run(8083);

    return 0;
}