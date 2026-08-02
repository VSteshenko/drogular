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