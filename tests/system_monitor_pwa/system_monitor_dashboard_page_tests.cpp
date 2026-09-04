#include "localization/system_monitor_translations.hpp"
#include "pages/dashboard_page.hpp"
#include "services/system_monitor.hpp"

#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

namespace {

class DashboardFakeProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshot() override {
        system_monitor::SystemSnapshot value;
        value.cpu.usagePercent = 25.0;
        value.cpu.logicalCores = 8;
        value.cpu.load1 = 0.5;
        value.cpu.load5 = 0.4;
        value.cpu.load15 = 0.3;
        value.memory.totalBytes = 16ULL * 1024 * 1024 * 1024;
        value.memory.usedBytes = 8ULL * 1024 * 1024 * 1024;
        value.memory.availableBytes = 8ULL * 1024 * 1024 * 1024;
        value.system.hostname = "monitor-test";
        value.system.operatingSystem = "macOS";
        value.system.kernel = "Darwin test";
        value.system.architecture = "arm64";
        value.system.uptimeSeconds = 90060;

        system_monitor::RaspberryPiHealth health;
        health.underVoltageOccurred = true;
        value.raspberryPi = system_monitor::RaspberryPiInfo{
            .model = "Raspberry Pi 4 Model B Rev 1.4",
            .revision = "c03114",
            .serial = "10000000a5bd2dd2",
            .temperatureCelsius = 38.946,
            .cpuFrequencyHz = 1500000000ULL,
            .health = health
        };

        system_monitor::DiskInfo disk;
        disk.device = "/dev/test";
        disk.mountPoint = "/";
        disk.fileSystem = "apfs";
        disk.totalBytes = 100ULL * 1024 * 1024 * 1024;
        disk.usedBytes = 25ULL * 1024 * 1024 * 1024;
        disk.availableBytes = 75ULL * 1024 * 1024 * 1024;
        value.disks.push_back(std::move(disk));
        return value;
    }

    std::vector<system_monitor::ProcessInfo> processes() override {
        return {};
    }
};

} // namespace

TEST(SystemMonitorDashboardPageTests, RendersSnapshotFromMonitorService) {
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

    auto provider = std::make_shared<DashboardFakeProvider>();
    services.registerService<system_monitor::SystemMonitor>(
        std::make_shared<system_monitor::SystemMonitor>(provider));

    const auto result =
        drogular::test::renderPage<system_monitor::DashboardPage>(&services);

    EXPECT_TRUE(drogular::test::contains(result.html, "monitor-test"));
    EXPECT_TRUE(drogular::test::contains(result.html, "25.0%"));
    EXPECT_TRUE(drogular::test::contains(result.html, "8 logical cores"));
    EXPECT_TRUE(drogular::test::contains(result.html, "8.00 GiB of 16.0 GiB used"));
    EXPECT_TRUE(drogular::test::contains(result.html, "1d 1h 1m"));
    EXPECT_TRUE(drogular::test::contains(result.html, "/dev/test"));
    EXPECT_TRUE(drogular::test::contains(result.html, "apfs"));
    EXPECT_TRUE(drogular::test::contains(result.html, "Darwin test"));
    EXPECT_TRUE(drogular::test::contains(result.html, "Raspberry Pi 4 Model B Rev 1.4"));
    EXPECT_TRUE(drogular::test::contains(result.html, "1500 MHz"));
    EXPECT_TRUE(drogular::test::contains(result.html, "38.9 °C"));
    EXPECT_TRUE(drogular::test::contains(result.html, "Events recorded"));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-monitor-field=\"cpu-usage\""));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-monitor-field=\"memory-usage\""));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-monitor-disk=\"/\""));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-monitor-status"));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-monitor-retry"));
    EXPECT_TRUE(drogular::test::contains(result.html, ">Retry</button>"));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-gpio-panel"));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-gpio-summary"));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-gpio-chips"));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-gpio-filters"));
    EXPECT_TRUE(drogular::test::contains(result.html, "data-gpio-filter=\"active\""));
    EXPECT_TRUE(drogular::test::contains(result.html, "aria-pressed=\"true\">Active</button>"));
    EXPECT_TRUE(drogular::test::contains(result.html, ">GPIO</h2>"));
}