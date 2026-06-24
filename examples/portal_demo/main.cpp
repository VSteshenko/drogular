#include "actions/language_action.hpp"
#include "pages/dashboard_page.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>

int main() {
    drogular::App app;

    app.templateRoot(
           "examples/portal_demo/templates"
       )
       .templateCache(false)
       .staticFileCacheProfile(
           drogular::StaticFileCacheProfile::Development
       );

    app.page<PortalDashboardPage>(
        "/dashboard"
    );

    app.action<PortalLanguageAction>(
        "/language"
    );

    app.run(8083);

    return 0;
}