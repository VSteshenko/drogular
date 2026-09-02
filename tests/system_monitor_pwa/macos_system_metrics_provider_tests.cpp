#if defined(__APPLE__)

#include "platform/macos_system_metrics_provider.hpp"

#include <gtest/gtest.h>

using namespace system_monitor;

TEST(MacOsSystemMetricsProviderTests, SnapshotContainsNativeSystemMetrics) {
    MacOsSystemMetricsProvider provider;

    const auto snapshot = provider.snapshot();

    EXPECT_GT(snapshot.cpu.logicalCores, 0U);
    EXPECT_GE(snapshot.cpu.usagePercent, 0.0);
    EXPECT_LE(snapshot.cpu.usagePercent, 100.0);
    EXPECT_GE(snapshot.cpu.load1, 0.0);
    EXPECT_GE(snapshot.cpu.load5, 0.0);
    EXPECT_GE(snapshot.cpu.load15, 0.0);

    EXPECT_GT(snapshot.memory.totalBytes, 0U);
    EXPECT_LE(snapshot.memory.usedBytes, snapshot.memory.totalBytes);
    EXPECT_LE(snapshot.memory.availableBytes, snapshot.memory.totalBytes);

    ASSERT_FALSE(snapshot.disks.empty());
    EXPECT_EQ(snapshot.disks.front().mountPoint, "/");
    EXPECT_GT(snapshot.disks.front().totalBytes, 0U);
    EXPECT_LE(snapshot.disks.front().usedBytes, snapshot.disks.front().totalBytes);
    EXPECT_LE(snapshot.disks.front().availableBytes, snapshot.disks.front().totalBytes);

    EXPECT_FALSE(snapshot.system.hostname.empty());
    EXPECT_EQ(snapshot.system.operatingSystem, "macOS");
    EXPECT_FALSE(snapshot.system.kernel.empty());
    EXPECT_FALSE(snapshot.system.architecture.empty());
    EXPECT_GT(snapshot.system.uptimeSeconds, 0U);

    EXPECT_FALSE(snapshot.raspberryPi.has_value());
}

TEST(MacOsSystemMetricsProviderTests, ProcessesContainNativeInventory) {
    MacOsSystemMetricsProvider provider;
    const auto processes = provider.processes();
    EXPECT_FALSE(processes.empty());
    EXPECT_GT(processes.front().pid, 0);
    EXPECT_FALSE(processes.front().name.empty());
}

#endif