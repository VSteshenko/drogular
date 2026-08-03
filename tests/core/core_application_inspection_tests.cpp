#include <drogular/app.hpp>
#include <drogular/component.hpp>
#include <drogular/page.hpp>
#include <drogular/action_handler.hpp>

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

    EXPECT_EQ(json["schemaVersion"].asInt(), 2);
    EXPECT_EQ(json["routes"][0]["kind"].asString(), "page");
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
    : public drogular::InspectionContributor
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
            std::move(data)
        });
    }
};

}

TEST(ApplicationInspectionTests, CollectsExtensionSectionsFromDiContributors) {
    drogular::App app;
    app.enableInspection();

    const auto contributors =
        app.services().service<drogular::InspectionContributors>();
    ASSERT_NE(contributors, nullptr);
    contributors->add(
        std::make_shared<CustomInspectionContributor>()
    );

    const auto inspection = app.inspect();
    ASSERT_EQ(inspection.sections.size(), 1u);
    EXPECT_EQ(inspection.sections.front().id, "authentication");

    const auto json = drogular::toJson(inspection);
    EXPECT_EQ(json["schemaVersion"].asInt(), 2);
    ASSERT_EQ(json["sections"].size(), 5u);
    EXPECT_EQ(json["sections"][4]["id"].asString(), "authentication");
    EXPECT_TRUE(json["sections"][4]["data"]["enabled"].asBool());
}

TEST(ApplicationInspectionTests, ReplacesSectionWithSameId) {
    drogular::ApplicationInspection inspection;
    inspection.addSection({"custom", "First", Json::Value(1)});
    inspection.addSection({"custom", "Second", Json::Value(2)});

    ASSERT_EQ(inspection.sections.size(), 1u);
    EXPECT_EQ(inspection.sections.front().title, "Second");
    EXPECT_EQ(inspection.sections.front().data.asInt(), 2);
}