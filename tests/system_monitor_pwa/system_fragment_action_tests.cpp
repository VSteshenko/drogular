#include "actions/system_fragment_action.hpp"
#include "localization/system_monitor_translations.hpp"
#include "services/system_monitor.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>
#include <drogon/HttpRequest.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

namespace {

class SystemFragmentProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshot() override {
        system_monitor::SystemSnapshot value;
        value.cpu.usagePercent = 42.5;
        value.cpu.logicalCores = 4;
        value.cpu.load1 = 0.25;
        value.cpu.load5 = 0.5;
        value.cpu.load15 = 0.75;
        value.memory.totalBytes = 8ULL * 1024 * 1024 * 1024;
        value.memory.usedBytes = 2ULL * 1024 * 1024 * 1024;
        value.memory.availableBytes = 6ULL * 1024 * 1024 * 1024;
        value.system.hostname = "fragment-host";
        value.system.operatingSystem = "Linux";
        value.system.kernel = "6.0-test";
        value.system.architecture = "aarch64";
        value.system.uptimeSeconds = 3660;
        value.disks.push_back({
            .device = "/dev/test",
            .mountPoint = "/",
            .fileSystem = "ext4",
            .totalBytes = 1000,
            .usedBytes = 250,
            .availableBytes = 750,
        });
        return value;
    }

    std::vector<system_monitor::ProcessInfo> processes() override { return {}; }
};

std::string render(bool german = false) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;
    options.setTemplateRoot(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates");
    services.setOptions(&options);
    services.expressionFunctions().registerFunction(
        "t", system_monitor::systemMonitorTranslationExpressionFunction());
    services.registerService<drogular::TranslationProvider>(
        std::make_shared<system_monitor::SystemMonitorTranslations>());
    services.registerService<system_monitor::SystemMonitor>(
        std::make_shared<system_monitor::SystemMonitor>(
            std::make_shared<SystemFragmentProvider>()));

    auto request = drogon::HttpRequest::newHttpRequest();
    if (german) request->addCookie("lang", "de");
    drogular::ActionContext context(request, &services);
    system_monitor::SystemFragmentAction action;

    return action.handle(context).body();
}

} // namespace

TEST(SystemFragmentActionTests, RendersLiveSnapshotAsServerHtml) {
    const auto html = render();

    EXPECT_NE(html.find("data-system-fragment"), std::string::npos);
    EXPECT_NE(html.find("data-dg-connection-state=\"live\""), std::string::npos);
    EXPECT_NE(html.find("fragment-host"), std::string::npos);
    EXPECT_NE(html.find("42.5%"), std::string::npos);
    EXPECT_NE(html.find("2.00 GiB of 8.00 GiB used"), std::string::npos);
    EXPECT_NE(html.find("/dev/test"), std::string::npos);
    EXPECT_NE(html.find("Host information"), std::string::npos);
}

TEST(SystemFragmentActionTests, UsesLanguageCookie) {
    const auto html = render(true);
    EXPECT_NE(html.find("Überwachungsziel"), std::string::npos);
    EXPECT_NE(html.find("Host-Informationen"), std::string::npos);
}