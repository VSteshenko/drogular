#include "actions/system_status_action.hpp"
#include "services/system_monitor.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

#include <memory>

namespace {

class StatusFakeProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshot() override {
        system_monitor::SystemSnapshot value;
        value.cpu.usagePercent = 12.5;
        value.cpu.logicalCores = 10;
        value.cpu.load1 = 1.1;
        value.cpu.load5 = 0.9;
        value.cpu.load15 = 0.7;
        value.memory.totalBytes = 1000;
        value.memory.usedBytes = 250;
        value.memory.availableBytes = 750;
        value.system.hostname = "api-test";
        value.system.operatingSystem = "macOS";
        value.system.kernel = "Darwin test";
        value.system.architecture = "arm64";
        value.system.uptimeSeconds = 3600;

        system_monitor::DiskInfo disk;
        disk.device = "/dev/test";
        disk.mountPoint = "/";
        disk.fileSystem = "apfs";
        disk.totalBytes = 2000;
        disk.usedBytes = 500;
        disk.availableBytes = 1500;
        value.disks.push_back(std::move(disk));

        return value;
    }

    std::vector<system_monitor::ProcessInfo> processes() override {
        return {};
    }
};

} // namespace

TEST(SystemMonitorStatusActionTests, ReturnsCurrentSnapshotAsJson) {
    drogular::ApplicationServices services;
    auto provider = std::make_shared<StatusFakeProvider>();
    services.registerService<system_monitor::SystemMonitor>(
        std::make_shared<system_monitor::SystemMonitor>(provider));

    auto request = drogon::HttpRequest::newHttpRequest();
    drogular::ActionContext context(request, &services);
    system_monitor::SystemStatusAction action;

    const auto result = action.handle(context);
    ASSERT_EQ(result.type(), drogular::ActionResultType::Json);

    const auto& json = result.json();
    EXPECT_GT(json["timestamp"].asInt64(), 0);
    EXPECT_DOUBLE_EQ(json["cpu"]["usagePercent"].asDouble(), 12.5);
    EXPECT_EQ(json["cpu"]["logicalCores"].asUInt(), 10U);
    EXPECT_DOUBLE_EQ(json["memory"]["usagePercent"].asDouble(), 25.0);
    EXPECT_EQ(json["system"]["hostname"].asString(), "api-test");
    EXPECT_EQ(json["system"]["uptimeSeconds"].asUInt64(), 3600U);
    ASSERT_EQ(json["disks"].size(), 1U);
    EXPECT_EQ(json["disks"][0]["mountPoint"].asString(), "/");
    EXPECT_DOUBLE_EQ(json["disks"][0]["usagePercent"].asDouble(), 25.0);

    ASSERT_TRUE(json.isMember("monitor"));
    EXPECT_EQ(json["monitor"]["updates"].asUInt64(), 1U);
    EXPECT_EQ(json["monitor"]["cacheHits"].asUInt64(), 0U);
    EXPECT_EQ(json["monitor"]["failedUpdates"].asUInt64(), 0U);
    EXPECT_EQ(json["monitor"]["refreshIntervalMs"].asInt64(), 1000);
    EXPECT_TRUE(json["monitor"]["healthy"].asBool());
    EXPECT_GT(json["monitor"]["lastSuccessfulUpdate"].asInt64(), 0);
}