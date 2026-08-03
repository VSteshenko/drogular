#include <drogular/app.hpp>
#include <drogular/developer_tools/diagnostics_page.hpp>
#include <drogular/developer_tools/application_inspection_controller.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <stdexcept>

namespace {

bool hasRoute(
    const drogular::ApplicationInspection& inspection,
    const std::string& path
) {
    return std::any_of(
        inspection.routes.begin(),
        inspection.routes.end(),
        [&](const auto& route) {
            return route.path == path;
        }
    );
}

TEST(ApplicationProfileTests, DefaultsToProductionWithoutDeveloperTools) {
    drogular::App app;

    EXPECT_EQ(app.profile(), drogular::ApplicationProfile::Production);

    const auto inspection = app.inspect();

    EXPECT_FALSE(hasRoute(inspection, drogular::DiagnosticsPage::Path));
    EXPECT_FALSE(hasRoute(
        inspection,
        drogular::ApplicationInspectionController::Path
    ));
}

TEST(ApplicationProfileTests, DevelopmentEnablesDeveloperTools) {
    drogular::App app;

    app.profile(drogular::ApplicationProfile::Development);

    const auto inspection = app.inspect();

    EXPECT_TRUE(hasRoute(inspection, drogular::DiagnosticsPage::Path));
    EXPECT_TRUE(hasRoute(
        inspection,
        drogular::ApplicationInspectionController::Path
    ));
}

TEST(ApplicationProfileTests, ExplicitDisableOverridesDevelopmentProfile) {
    drogular::App app;

    app.disableDeveloperTools()
        .profile(drogular::ApplicationProfile::Development);

    const auto inspection = app.inspect();

    EXPECT_FALSE(hasRoute(inspection, drogular::DiagnosticsPage::Path));
}

TEST(ApplicationProfileTests, CannotDisableAfterRegistration) {
    drogular::App app;
    app.enableDeveloperTools();

    EXPECT_THROW(
        app.disableDeveloperTools(),
        std::logic_error
    );
}

} // namespace