#include "actions/language_action.hpp"
#include "pages/dashboard_page.hpp"
#include "pages/login_page.hpp"
#include "actions/login_action.hpp"
#include "actions/logout_action.hpp"
#include "pages/users_page.hpp"

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
    );

    app.services().add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    app.page<PortalLoginPage>("/");
    app.page<PortalLoginPage>("/login");
    app.page<PortalDashboardPage>("/dashboard");
    app.page<PortalUsersPage>("/users");

    app.action<PortalLanguageAction>("/language");
    app.action<PortalLoginAction>("/login");
    app.action<PortalLogoutAction>("/logout");

    app.run(8083);

    return 0;
}
