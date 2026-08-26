#include "services/system_monitor.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

namespace {

class FakeMetricsProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshotValue;
    std::vector<system_monitor::ProcessInfo> processValues;
    int snapshotCalls{0};
    int processCalls{0};
    bool throwOnSnapshot{false};

    system_monitor::SystemSnapshot snapshot() override {
        ++snapshotCalls;
        if (throwOnSnapshot) {
            throw std::runtime_error("snapshot failed");
        }
        return snapshotValue;
    }

    std::vector<system_monitor::ProcessInfo> processes() override {
        ++processCalls;
        return processValues;
    }
};

class SlowMetricsProvider final
    : public system_monitor::SystemMetricsProvider
{
public:
    std::atomic<int> snapshotCalls{0};

    system_monitor::SystemSnapshot snapshot() override {
        ++snapshotCalls;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        system_monitor::SystemSnapshot value;
        value.system.hostname = "shared-snapshot";
        return value;
    }

    std::vector<system_monitor::ProcessInfo> processes() override {
        return {};
    }
};

} // namespace

TEST(SystemMonitorServiceTests, DelegatesFirstSnapshotToProvider) {
    auto provider = std::make_shared<FakeMetricsProvider>();
    provider->snapshotValue.system.hostname = "test-host";
    provider->snapshotValue.cpu.logicalCores = 8;

    system_monitor::SystemMonitor monitor(provider);
    const auto snapshot = monitor.snapshot();

    EXPECT_EQ(provider->snapshotCalls, 1);
    EXPECT_EQ(snapshot.system.hostname, "test-host");
    EXPECT_EQ(snapshot.cpu.logicalCores, 8U);

    const auto statistics = monitor.statistics();
    EXPECT_EQ(statistics.updates, 1U);
    EXPECT_EQ(statistics.cacheHits, 0U);
    EXPECT_EQ(statistics.failedUpdates, 0U);
    EXPECT_TRUE(statistics.healthy);
    EXPECT_EQ(statistics.refreshInterval, std::chrono::seconds(1));
}

TEST(SystemMonitorServiceTests, ReusesSnapshotWithinRefreshInterval) {
    auto provider = std::make_shared<FakeMetricsProvider>();
    provider->snapshotValue.system.hostname = "first";

    system_monitor::SystemMonitor monitor(provider, std::chrono::seconds(30));
    const auto first = monitor.snapshot();

    provider->snapshotValue.system.hostname = "second";
    const auto second = monitor.snapshot();

    EXPECT_EQ(provider->snapshotCalls, 1);
    EXPECT_EQ(first.system.hostname, "first");
    EXPECT_EQ(second.system.hostname, "first");

    const auto statistics = monitor.statistics();
    EXPECT_EQ(statistics.updates, 1U);
    EXPECT_EQ(statistics.cacheHits, 1U);
    EXPECT_TRUE(statistics.healthy);
}

TEST(SystemMonitorServiceTests, RefreshesWhenIntervalIsZero) {
    auto provider = std::make_shared<FakeMetricsProvider>();
    provider->snapshotValue.system.hostname = "first";

    system_monitor::SystemMonitor monitor(provider, std::chrono::milliseconds::zero());
    const auto first = monitor.snapshot();

    provider->snapshotValue.system.hostname = "second";
    const auto second = monitor.snapshot();

    EXPECT_EQ(provider->snapshotCalls, 2);
    EXPECT_EQ(first.system.hostname, "first");
    EXPECT_EQ(second.system.hostname, "second");

    const auto statistics = monitor.statistics();
    EXPECT_EQ(statistics.updates, 2U);
    EXPECT_EQ(statistics.cacheHits, 0U);
}

TEST(SystemMonitorServiceTests, ReturnsStaleSnapshotWhenRefreshFails) {
    auto provider = std::make_shared<FakeMetricsProvider>();
    provider->snapshotValue.system.hostname = "last-good";

    system_monitor::SystemMonitor monitor(provider, std::chrono::milliseconds::zero());
    const auto first = monitor.snapshot();
    ASSERT_EQ(first.system.hostname, "last-good");

    provider->throwOnSnapshot = true;
    EXPECT_NO_THROW({
        const auto stale = monitor.snapshot();
        EXPECT_EQ(stale.system.hostname, "last-good");
    });

    EXPECT_EQ(provider->snapshotCalls, 2);
    const auto statistics = monitor.statistics();
    EXPECT_EQ(statistics.updates, 1U);
    EXPECT_EQ(statistics.failedUpdates, 1U);
    EXPECT_FALSE(statistics.healthy);
}

TEST(SystemMonitorServiceTests, PropagatesInitialRefreshFailure) {
    auto provider = std::make_shared<FakeMetricsProvider>();
    provider->throwOnSnapshot = true;

    system_monitor::SystemMonitor monitor(provider);

    EXPECT_THROW(monitor.snapshot(), std::runtime_error);
    const auto statistics = monitor.statistics();
    EXPECT_EQ(statistics.updates, 0U);
    EXPECT_EQ(statistics.failedUpdates, 1U);
    EXPECT_FALSE(statistics.healthy);
}

TEST(SystemMonitorServiceTests, SharesOneRefreshAcrossConcurrentReaders) {
    auto provider = std::make_shared<SlowMetricsProvider>();
    system_monitor::SystemMonitor monitor(provider, std::chrono::seconds(30));

    constexpr std::size_t readerCount = 8;
    std::vector<system_monitor::SystemSnapshot> snapshots(readerCount);
    std::vector<std::thread> readers;
    readers.reserve(readerCount);

    for (std::size_t index = 0; index < readerCount; ++index) {
        readers.emplace_back([&monitor, &snapshots, index] {
            snapshots[index] = monitor.snapshot();
        });
    }
    for (auto& reader : readers) {
        reader.join();
    }

    EXPECT_EQ(provider->snapshotCalls.load(), 1);
    for (const auto& snapshot : snapshots) {
        EXPECT_EQ(snapshot.system.hostname, "shared-snapshot");
    }

    const auto statistics = monitor.statistics();
    EXPECT_EQ(statistics.updates, 1U);
    EXPECT_EQ(statistics.cacheHits, readerCount - 1);
}

TEST(SystemMonitorServiceTests, DelegatesProcessesToProvider) {
    auto provider = std::make_shared<FakeMetricsProvider>();
    system_monitor::ProcessInfo process;
    process.pid = 42;
    process.name = "worker";
    provider->processValues.push_back(std::move(process));

    system_monitor::SystemMonitor monitor(provider);
    const auto processes = monitor.processes();

    EXPECT_EQ(provider->processCalls, 1);
    ASSERT_EQ(processes.size(), 1U);
    EXPECT_EQ(processes.front().pid, 42);
    EXPECT_EQ(processes.front().name, "worker");
}

TEST(SystemMonitorServiceTests, RejectsNullProvider) {
    EXPECT_THROW(
        system_monitor::SystemMonitor(nullptr),
        std::invalid_argument);
}

TEST(SystemMonitorServiceTests, RejectsNegativeRefreshInterval) {
    auto provider = std::make_shared<FakeMetricsProvider>();

    EXPECT_THROW(
        system_monitor::SystemMonitor(provider, std::chrono::milliseconds(-1)),
        std::invalid_argument);
}