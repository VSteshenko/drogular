#include "home_page.hpp"
#include "system_info_contributor.hpp"

#include <drogular/app.hpp>

#include <memory>

int main() {
    drogular::App app;

    app.profile(drogular::ApplicationProfile::Development)
        .staticFiles(
            "/assets",
            "examples/developer_tools/public"
        )
        .page<DeveloperToolsExamplePage>("/")
        .developerToolsContributor(
            std::make_shared<SystemInfoContributor>()
        )
        .developerToolsComponent(
            "example.system-info",
            "/assets/system-info.js"
        );

    app.run(8082);
    return 0;
}