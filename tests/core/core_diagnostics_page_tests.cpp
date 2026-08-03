#include <drogular/app.hpp>
#include <drogular/developer_tools/diagnostics_page.hpp>
#include <drogular/developer_tools/diagnostics_resources.hpp>
#include <drogular/render_context.hpp>

#include <gtest/gtest.h>

#include <algorithm>

namespace {

TEST(DiagnosticsPageTests, RendersIndependentInspectionClient) {
    drogular::DiagnosticsPage page;
    drogular::RenderContext context;

    const auto html = page.render(context);

    EXPECT_NE(html.find("/__drogular/inspection"), std::string::npos);
    EXPECT_NE(html.find("/__drogular/assets/diagnostics.css"), std::string::npos);
    EXPECT_NE(html.find("/__drogular/assets/diagnostics.js"), std::string::npos);
    EXPECT_EQ(html.find("ApplicationInspectionProvider"), std::string::npos);
}

TEST(DiagnosticsPageTests, ShipsBrowserAssets) {
    const auto css =
        drogular::diagnostics_resources::stylesheet();
    const auto script =
        drogular::diagnostics_resources::script();

    EXPECT_FALSE(css.empty());
    EXPECT_NE(script.find("fetch(inspectionEndpoint"), std::string_view::npos);
    EXPECT_NE(script.find("/__drogular/inspection"), std::string_view::npos);
    EXPECT_NE(script.find("componentsEndpoint"), std::string_view::npos);
    EXPECT_NE(script.find("import(moduleUrl)"), std::string_view::npos);
}

TEST(DiagnosticsPageTests, EnablingPageAlsoEnablesInspectionContract) {
    drogular::App app;

    app.enableDiagnosticsPage();

    const auto inspection = app.inspect();
    const auto hasDiagnosticsPage = std::any_of(
        inspection.routes.begin(),
        inspection.routes.end(),
        [](const auto& route) {
            return route.path == drogular::DiagnosticsPage::Path;
        }
    );
    const auto hasInspection = std::any_of(
        inspection.routes.begin(),
        inspection.routes.end(),
        [](const auto& route) {
            return route.path == drogular::ApplicationInspectionController::Path;
        }
    );

    EXPECT_TRUE(hasDiagnosticsPage);
    EXPECT_TRUE(hasInspection);
    EXPECT_NE(
        app.services().service<drogular::ApplicationInspectionProvider>(),
        nullptr
    );
}

} // namespace