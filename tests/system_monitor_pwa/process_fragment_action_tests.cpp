#include "actions/process_fragment_action.hpp"
#include "localization/system_monitor_translations.hpp"
#include "services/process_service.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>
#include <drogular/translation_provider.hpp>
#include <drogon/HttpRequest.h>

#include <gtest/gtest.h>

#include <filesystem>

namespace {

class ProcessFragmentProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshot() override { return {}; }

    std::vector<system_monitor::ProcessInfo> processes() override {
        return {
            {.pid = 20, .user = "bob", .name = "idle", .command = "/usr/bin/idle",
             .cpuPercent = 1.0, .memoryPercent = 4.0, .residentBytes = 8192},
            {.pid = 10, .user = "alice", .name = "worker", .command = "/usr/bin/worker --serve",
             .cpuPercent = 8.5, .memoryPercent = 1.5, .residentBytes = 4096},
        };
    }
};

void configureServices(
    drogular::ApplicationServices& services,
    drogular::ApplicationOptions& options) {
    options.setTemplateRoot(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates");
    services.setOptions(&options);
    services.expressionFunctions().registerFunction(
        "t", system_monitor::systemMonitorTranslationExpressionFunction());
    services.registerService<drogular::TranslationProvider>(
        std::make_shared<system_monitor::SystemMonitorTranslations>());
    services.registerService<system_monitor::ProcessService>(
        std::make_shared<system_monitor::ProcessService>(
            std::make_shared<ProcessFragmentProvider>()));
}

} // namespace

TEST(ProcessFragmentActionTests, RendersLocalizedHtmlInsteadOfClientSideRows) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;
    configureServices(services, options);
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setParameter("query", "worker");
    request->setParameter("sort", "name");
    drogular::ActionContext context(request, &services);
    system_monitor::ProcessFragmentAction action;

    const auto result = action.handle(context);
    const auto html = result.body();

    EXPECT_NE(html.find("worker"), std::string::npos);
    EXPECT_NE(html.find("alice"), std::string::npos);
    EXPECT_EQ(html.find("idle"), std::string::npos);
    EXPECT_NE(html.find("1 / 1 matching"), std::string::npos);
    EXPECT_NE(html.find("8.5%"), std::string::npos);
}

TEST(ProcessFragmentActionTests, MarksEmptyResultForDeclarativeRuntime) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;
    configureServices(services, options);
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setParameter("query", "missing");
    drogular::ActionContext context(request, &services);
    system_monitor::ProcessFragmentAction action;

    const auto html = action.handle(context).body();

    EXPECT_NE(html.find("data-dg-empty"), std::string::npos);
    EXPECT_NE(html.find("dg-empty-state"), std::string::npos);
    EXPECT_NE(html.find("No matching processes."), std::string::npos);
    EXPECT_EQ(html.find("<table"), std::string::npos);
}

TEST(ProcessFragmentActionTests, UsesLanguageCookieForServerRenderedFragment) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;
    configureServices(services, options);
    auto request = drogon::HttpRequest::newHttpRequest();
    request->addCookie("lang", "de");
    request->setParameter("query", "missing");
    drogular::ActionContext context(request, &services);
    system_monitor::ProcessFragmentAction action;

    const auto html = action.handle(context).body();

    EXPECT_NE(html.find("Keine passenden Prozesse."), std::string::npos);
}