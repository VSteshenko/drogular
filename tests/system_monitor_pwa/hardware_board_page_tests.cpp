#include "localization/system_monitor_translations.hpp"
#include "pages/board_page.hpp"
#include "services/system_monitor.hpp"

#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

namespace {

class BoardPageFakeProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshot() override {
        system_monitor::SystemSnapshot value;
        value.system.hostname = "pi-monitor";
        value.system.operatingSystem = "Linux";
        value.system.architecture = "aarch64";
        value.raspberryPi = system_monitor::RaspberryPiInfo{
            .model = "Raspberry Pi 4 Model B Rev 1.4",
            .revision = "c03114",
            .serial = "10000000a5bd2dd2"
        };

        return value;
    }

    std::vector<system_monitor::ProcessInfo> processes() override {
        return {};
    }
};

class GenericLinuxFakeProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshot() override {
        system_monitor::SystemSnapshot value;
        value.system.hostname = "linux-monitor";
        value.system.operatingSystem = "Linux";
        value.system.architecture = "x86_64";
        return value;
    }

    std::vector<system_monitor::ProcessInfo> processes() override {
        return {};
    }
};

} // namespace

TEST(HardwareBoardPageTests, RendersBoardIdentityAndHardwareShell) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;
    options.setTemplateRoot(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates");
    services.setOptions(&options);
    services.expressionFunctions().registerFunction(
        "t",
        system_monitor::systemMonitorTranslationExpressionFunction());
    services.registerService<drogular::TranslationProvider>(
        std::make_shared<system_monitor::SystemMonitorTranslations>());

    auto provider = std::make_shared<BoardPageFakeProvider>();
    services.registerService<system_monitor::SystemMonitor>(
        std::make_shared<system_monitor::SystemMonitor>(provider));

    const auto result =
        drogular::test::renderPage<system_monitor::BoardPage>(&services);

    EXPECT_TRUE(drogular::test::contains(result.html, "Drogular Hardware Overview"));
    EXPECT_TRUE(drogular::test::contains(result.html, "Raspberry Pi 4 Model B Rev 1.4"));
    EXPECT_TRUE(drogular::test::contains(result.html, "c03114"));
    EXPECT_TRUE(drogular::test::contains(result.html, "10000000a5bd2dd2"));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-board-header-map"));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-board-interface-list"));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-board-summary=\"gpio\""));
    EXPECT_TRUE(drogular::test::contains(result.html, "/assets/board.js"));
    EXPECT_TRUE(drogular::test::contains(result.html, "class=\"secondary-link\" href=\"/\""));
}
TEST(HardwareBoardPageTests, GenericLinuxDoesNotRenderRaspberryPiSpecification) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;
    options.setTemplateRoot(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates");
    services.setOptions(&options);
    services.expressionFunctions().registerFunction(
        "t",
        system_monitor::systemMonitorTranslationExpressionFunction());
    services.registerService<drogular::TranslationProvider>(
        std::make_shared<system_monitor::SystemMonitorTranslations>());

    auto provider = std::make_shared<GenericLinuxFakeProvider>();
    services.registerService<system_monitor::SystemMonitor>(
        std::make_shared<system_monitor::SystemMonitor>(provider));

    const auto result =
        drogular::test::renderPage<system_monitor::BoardPage>(&services);

    EXPECT_TRUE(drogular::test::contains(result.html, "Linux · x86_64"));
    EXPECT_TRUE(drogular::test::contains(result.html, "linux-monitor · hardware interfaces detected on this monitoring target"));
    EXPECT_FALSE(drogular::test::contains(result.html, "40-pin header"));
    EXPECT_FALSE(drogular::test::contains(result.html, "data-board-header-map"));
    EXPECT_FALSE(drogular::test::contains(result.html, "Revision</dt>"));
    EXPECT_FALSE(drogular::test::contains(result.html, "Serial</dt>"));
    EXPECT_TRUE(drogular::test::contains(result.html, "without applying any board layout assumptions"));
}