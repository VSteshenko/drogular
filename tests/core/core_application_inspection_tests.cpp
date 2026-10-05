#include <drogular/app.hpp>
#include <drogular/component.hpp>
#include <drogular/page.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/developer_tools/runtime_diagnostics_contributor.hpp>

#include <gtest/gtest.h>

namespace {

class InspectionPage final
    : public drogular::Page
{
public:
    std::string render(
        drogular::RenderContext&
    ) override {
        return {};
    }
};

class InspectionAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext&
    ) override {
        return drogular::ActionResult::empty();
    }
};

class InspectionComponent final
    : public drogular::Component
{
public:
    std::string render(
        drogular::RenderContext&
    ) override {
        return {};
    }
};

struct InspectionService {};

}

TEST(ApplicationInspectionTests, ReportsConfiguredApplicationSurface) {
    drogular::App app;
    app.page<InspectionPage>("/dashboard")
       .action<InspectionAction>("/save")
       .component<InspectionComponent>("inspection-card")
       .staticFiles("/assets", "public")
       .serviceWorker("public/service-worker.js")
       .offlinePage<InspectionPage>();

    app.services().addLazy<InspectionService>(
        [] {
            return std::make_shared<InspectionService>();
        }
    );

    const auto inspection = app.inspect();

    EXPECT_EQ(inspection.routes.size(), 5u);
    ASSERT_EQ(inspection.components.size(), 1u);
    EXPECT_EQ(inspection.components.front().tag, "inspection-card");
    ASSERT_EQ(inspection.services.size(), 1u);
    EXPECT_EQ(inspection.services.front().lifetime, drogular::ServiceLifetime::LazySingleton);
    EXPECT_FALSE(inspection.services.front().instantiated);
}
TEST(ApplicationInspectionTests, SerializesStableJsonContract) {
    drogular::ApplicationInspection inspection;
    inspection.routes.push_back({
        "/",
        drogular::RouteKind::Page,
        "GET",
        "HomePage"
    });
    inspection.components.push_back({
        "app-card"
    });
    inspection.services.push_back({
        "ExampleService",
        drogular::ServiceLifetime::Scoped,
        false
    });
    inspection.diagnostics.push_back({
        "DGL-CMP-001",
        drogular::DiagnosticSeverity::Warning,
        "Duplicate component",
        {"app.cpp", 10, 2, 3}
    });

    const auto json = drogular::toJson(inspection);

    EXPECT_EQ(json["schemaVersion"].asInt(), 3);
    EXPECT_EQ(json["routes"][0]["kind"].asString(), "page");
    EXPECT_EQ(json["sections"][0]["component"].asString(), "drogular.routes");
    EXPECT_EQ(json["components"][0]["tag"].asString(), "app-card");
    EXPECT_EQ(json["services"][0]["lifetime"].asString(), "scoped");
    EXPECT_EQ(json["diagnostics"][0]["severity"].asString(), "warning");
    EXPECT_EQ(json["diagnostics"][0]["location"]["line"].asUInt64(), 2u);
}

TEST(ApplicationInspectionTests, RegistersProviderThroughDependencyInjection) {
    drogular::App app;
    app.enableInspection();

    const auto provider =
        app.services().service<drogular::ApplicationInspectionProvider>();

    ASSERT_NE(provider, nullptr);
    const auto inspection = (*provider)();

    ASSERT_EQ(inspection.routes.size(), 1u);
    EXPECT_EQ(inspection.routes.front().kind, drogular::RouteKind::Inspection);
    EXPECT_EQ(inspection.routes.front().path, "/__drogular/inspection");
}

namespace {

class CustomInspectionContributor final
    : public drogular::DeveloperToolsContributor
{
public:
    void contribute(
        drogular::ApplicationInspection& inspection
    ) const override {
        Json::Value data(Json::objectValue);
        data["enabled"] = true;
        data["provider"] = "example";
        inspection.addSection({
            "authentication",
            "Authentication",
            "example.authentication",
            std::move(data)
        });
    }
};

}

TEST(ApplicationInspectionTests, CollectsExtensionSectionsFromDiContributors) {
    drogular::App app;
    app.enableInspection();

    const auto contributors =
        app.services().service<drogular::DeveloperToolsContributors>();
    ASSERT_NE(contributors, nullptr);
    contributors->add(
        std::make_shared<CustomInspectionContributor>()
    );

    const auto inspection = app.inspect();
    ASSERT_EQ(inspection.sections.size(), 4u);
    EXPECT_EQ(inspection.sections.back().id, "authentication");

    const auto json = drogular::toJson(inspection);
    EXPECT_EQ(json["schemaVersion"].asInt(), 3);
    ASSERT_EQ(json["sections"].size(), 8u);
    EXPECT_EQ(json["sections"][7]["id"].asString(), "authentication");
    EXPECT_EQ(json["sections"][7]["component"].asString(), "example.authentication");
    EXPECT_TRUE(json["sections"][7]["data"]["enabled"].asBool());
}

TEST(ApplicationInspectionTests, ReplacesSectionWithSameId) {
    drogular::ApplicationInspection inspection;
    inspection.addSection({
        "custom",
        "First",
        "example.first",
        Json::Value(1)
    });
    inspection.addSection({
        "custom",
        "Second",
        "example.second",
        Json::Value(2)
    });

    ASSERT_EQ(inspection.sections.size(), 1u);
    EXPECT_EQ(inspection.sections.front().title, "Second");
    EXPECT_EQ(inspection.sections.front().component, "example.second");
    EXPECT_EQ(inspection.sections.front().data.asInt(), 2);
}

TEST(ApplicationInspectionTests, ReportsGetActionMethod) {
    drogular::App app;
    app.get<InspectionAction>("/api/status");

    const auto inspection = app.inspect();

    ASSERT_EQ(inspection.routes.size(), 1u);
    EXPECT_EQ(inspection.routes.front().kind, drogular::RouteKind::Action);
    EXPECT_EQ(inspection.routes.front().method, "GET");
    EXPECT_EQ(inspection.routes.front().path, "/api/status");
}

TEST(ApplicationInspectionTests, ReportsRuntimeDiagnosticsSections) {
    drogular::RuntimeDiagnostics diagnostics;
    diagnostics.recordRequest();
    diagnostics.recordAction();
    diagnostics.recordRenderedAction();
    diagnostics.recordRedirect();
    diagnostics.recordInteractionRequest();
    diagnostics.recordInteractionGetRequest();
    diagnostics.recordInteractionSuccessfulResponse();
    diagnostics.recordComponentRendered(3);
    diagnostics.recordRenderFailure();
    diagnostics.recordServiceScopeResolution();
    diagnostics.recordServiceScopeCacheHit();
    diagnostics.recordServiceScopeServiceCreated();

    drogular::ApplicationInspection inspection;
    drogular::RuntimeDiagnosticsContributor contributor(diagnostics);
    contributor.contribute(inspection);

    ASSERT_EQ(inspection.sections.size(), 3u);

    const auto& runtime = inspection.sections[0];
    EXPECT_EQ(runtime.id, "runtime");
    EXPECT_EQ(runtime.component, "drogular.runtime");
    EXPECT_EQ(runtime.data["requests"]["requests"].asUInt64(), 1u);
    EXPECT_EQ(runtime.data["requests"]["actions"].asUInt64(), 1u);
    EXPECT_EQ(runtime.data["requests"]["renderedActions"].asUInt64(), 1u);
    EXPECT_EQ(runtime.data["requests"]["redirects"].asUInt64(), 1u);
    EXPECT_EQ(runtime.data["rendering"]["componentsRendered"].asUInt64(), 1u);
    EXPECT_EQ(runtime.data["rendering"]["renderFailures"].asUInt64(), 1u);
    EXPECT_EQ(runtime.data["rendering"]["maximumDepth"].asUInt64(), 3u);

    const auto& interactions = inspection.sections[1];
    EXPECT_EQ(interactions.id, "interactions");
    EXPECT_EQ(interactions.component, "drogular.interactions");
    EXPECT_EQ(interactions.data["requests"].asUInt64(), 1u);
    EXPECT_EQ(interactions.data["getRequests"].asUInt64(), 1u);
    EXPECT_EQ(interactions.data["postRequests"].asUInt64(), 0u);
    EXPECT_EQ(interactions.data["successfulResponses"].asUInt64(), 1u);
    EXPECT_EQ(interactions.data["clientErrorResponses"].asUInt64(), 0u);
    EXPECT_EQ(interactions.data["redirects"].asUInt64(), 0u);

    const auto& scopes = inspection.sections[2];
    EXPECT_EQ(scopes.id, "service-scopes");
    EXPECT_EQ(scopes.component, "drogular.service-scopes");
    EXPECT_EQ(scopes.data["resolutions"].asUInt64(), 1u);
    EXPECT_EQ(scopes.data["cacheHits"].asUInt64(), 1u);
    EXPECT_EQ(scopes.data["servicesCreated"].asUInt64(), 1u);
}

TEST(ApplicationInspectionTests, AppInspectionIncludesRuntimeDiagnosticsSections) {
    drogular::App app;

    const auto inspection = app.inspect();

    ASSERT_EQ(inspection.sections.size(), 3u);
    EXPECT_EQ(inspection.sections[0].id, "runtime");
    EXPECT_EQ(inspection.sections[1].id, "interactions");
    EXPECT_EQ(inspection.sections[2].id, "service-scopes");
}