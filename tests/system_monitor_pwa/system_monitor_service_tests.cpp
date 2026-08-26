#include "services/system_monitor.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <utility>

namespace {

class FakeMetricsProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshotValue;
    std::vector<system_monitor::ProcessInfo> processValues;
    int snapshotCalls{0};
    int processCalls{0};

    system_monitor::SystemSnapshot snapshot() override {
        ++snapshotCalls;
        return snapshotValue;
    }

    std::vector<system_monitor::ProcessInfo> processes() override {
        ++processCalls;
        return processValues;
    }
};

} // namespace

TEST(SystemMonitorServiceTests, DelegatesSnapshotToProvider) {
    auto provider = std::make_shared<FakeMetricsProvider>();
    provider->snapshotValue.system.hostname = "test-host";
    provider->snapshotValue.cpu.logicalCores = 8;

    system_monitor::SystemMonitor monitor(provider);
    const auto snapshot = monitor.snapshot();

    EXPECT_EQ(provider->snapshotCalls, 1);
    EXPECT_EQ(snapshot.system.hostname, "test-host");
    EXPECT_EQ(snapshot.cpu.logicalCores, 8U);
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