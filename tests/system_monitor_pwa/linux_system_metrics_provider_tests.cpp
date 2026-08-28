#include "platform/linux_system_metrics_provider.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace {

class FakeLinuxReader final : public system_monitor::SystemReader {
public:
    std::unordered_map<std::string, std::string> files;
    std::unordered_map<std::string, system_monitor::CommandResult> commands;

    std::string readFile(std::string_view path) override {
        const auto it = files.find(std::string(path));
        if (it == files.end()) {
            throw std::runtime_error("missing fake file");
        }
        return it->second;
    }

    system_monitor::CommandResult execute(std::string_view command) override {
        const auto it = commands.find(std::string(command));
        if (it == commands.end()) {
            throw std::runtime_error("missing fake command");
        }
        return it->second;
    }
};

std::shared_ptr<FakeLinuxReader> makeReader() {
    auto reader = std::make_shared<FakeLinuxReader>();
    reader->files["/proc/cpuinfo"] =
        "processor : 0\nmodel name : Test CPU\nprocessor : 1\n";
    reader->files["/proc/loadavg"] = "0.50 0.25 0.10 1/100 42\n";
    reader->files["/proc/stat"] = "cpu 100 10 40 800 20 5 5 0 0 0\n";
    reader->files["/proc/meminfo"] =
        "MemTotal:       1000000 kB\n"
        "MemAvailable:    400000 kB\n"
        "MemFree:         100000 kB\n";
    reader->files["/proc/uptime"] = "12345.67 100.0\n";
    reader->commands["df -P -B1 /"] = {0,
        "Filesystem 1-blocks Used Available Capacity Mounted on\n"
        "/dev/test 1000000 600000 400000 60% /\n", {}};
    reader->commands["hostname"] = {0, "test-linux\n", {}};
    reader->commands["uname -s"] = {0, "Linux\n", {}};
    reader->commands["uname -r"] = {0, "6.8.0-test\n", {}};
    reader->commands["uname -m"] = {0, "aarch64\n", {}};

    return reader;
}

} // namespace

TEST(LinuxSystemMetricsProviderTests, SnapshotParsesLinuxReaderData) {
    auto reader = makeReader();
    system_monitor::LinuxSystemMetricsProvider provider(reader);

    const auto snapshot = provider.snapshot();

    EXPECT_EQ(snapshot.cpu.logicalCores, 2U);
    EXPECT_GT(snapshot.cpu.usagePercent, 0.0);
    EXPECT_DOUBLE_EQ(snapshot.cpu.load1, 0.50);
    EXPECT_DOUBLE_EQ(snapshot.cpu.load5, 0.25);
    EXPECT_DOUBLE_EQ(snapshot.cpu.load15, 0.10);

    EXPECT_EQ(snapshot.memory.totalBytes, 1000000ULL * 1024ULL);
    EXPECT_EQ(snapshot.memory.availableBytes, 400000ULL * 1024ULL);
    EXPECT_EQ(snapshot.memory.usedBytes, 600000ULL * 1024ULL);

    ASSERT_EQ(snapshot.disks.size(), 1U);
    EXPECT_EQ(snapshot.disks[0].device, "/dev/test");
    EXPECT_EQ(snapshot.disks[0].mountPoint, "/");
    EXPECT_EQ(snapshot.disks[0].totalBytes, 1000000U);
    EXPECT_EQ(snapshot.disks[0].usedBytes, 600000U);
    EXPECT_EQ(snapshot.disks[0].availableBytes, 400000U);

    EXPECT_EQ(snapshot.system.hostname, "test-linux");
    EXPECT_EQ(snapshot.system.operatingSystem, "Linux");
    EXPECT_EQ(snapshot.system.kernel, "Linux 6.8.0-test");
    EXPECT_EQ(snapshot.system.architecture, "aarch64");
    EXPECT_EQ(snapshot.system.uptimeSeconds, 12345U);
    EXPECT_FALSE(snapshot.raspberryPi.has_value());
}

TEST(LinuxSystemMetricsProviderTests, AddsRaspberryPiProbeDataToSnapshot) {
    auto reader = makeReader();
    reader->files["/proc/cpuinfo"] =
        "processor : 0\n"
        "processor : 1\n"
        "processor : 2\n"
        "processor : 3\n"
        "Model : Raspberry Pi 4 Model B Rev 1.4\n"
        "Revision : c03114\n"
        "Serial : 10000000a5bd2dd2\n";
    reader->commands["cat /sys/class/thermal/thermal_zone0/temp"] =
        {0, "38946\n", {}};
    reader->commands[
        "cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"] =
        {1, {}, "not available"};
    reader->commands["vcgencmd get_throttled"] =
        {1, {}, "not available"};

    system_monitor::LinuxSystemMetricsProvider provider(reader);
    const auto snapshot = provider.snapshot();

    ASSERT_TRUE(snapshot.raspberryPi.has_value());
    EXPECT_EQ(snapshot.raspberryPi->model, "Raspberry Pi 4 Model B Rev 1.4");
    ASSERT_TRUE(snapshot.cpu.temperatureCelsius.has_value());
    EXPECT_DOUBLE_EQ(*snapshot.cpu.temperatureCelsius, 38.946);
}

TEST(LinuxSystemMetricsProviderTests, CpuUsageUsesDeltaBetweenSnapshots) {
    auto reader = makeReader();
    system_monitor::LinuxSystemMetricsProvider provider(reader);

    (void)provider.snapshot();
    reader->files["/proc/stat"] = "cpu 120 10 50 850 20 5 5 0 0 0\n";

    const auto snapshot = provider.snapshot();
    EXPECT_NEAR(snapshot.cpu.usagePercent, 37.5, 0.001);
}

TEST(LinuxSystemMetricsProviderTests, UsesFallbackWhenMemAvailableIsMissing) {
    auto reader = makeReader();
    reader->files["/proc/meminfo"] =
        "MemTotal:       1000 kB\n"
        "MemFree:         100 kB\n"
        "Buffers:          50 kB\n"
        "Cached:          200 kB\n"
        "SReclaimable:     25 kB\n";
    system_monitor::LinuxSystemMetricsProvider provider(reader);

    const auto snapshot = provider.snapshot();
    EXPECT_EQ(snapshot.memory.availableBytes, 375ULL * 1024ULL);
    EXPECT_EQ(snapshot.memory.usedBytes, 625ULL * 1024ULL);
}

TEST(LinuxSystemMetricsProviderTests, RejectsNullReader) {
    EXPECT_THROW(system_monitor::LinuxSystemMetricsProvider(nullptr), std::invalid_argument);
}

TEST(LinuxSystemMetricsProviderTests, ProcessesAreDeferredToDedicatedStage) {
    auto reader = makeReader();
    system_monitor::LinuxSystemMetricsProvider provider(reader);
    EXPECT_TRUE(provider.processes().empty());
}

#if defined(__linux__)
#include "platform/local_linux_system_reader.hpp"

TEST(LinuxSystemMetricsProviderTests, NativeSnapshotContainsSystemMetrics) {
    auto reader = std::make_shared<system_monitor::LocalLinuxSystemReader>();
    system_monitor::LinuxSystemMetricsProvider provider(reader);

    const auto snapshot = provider.snapshot();
    EXPECT_GT(snapshot.cpu.logicalCores, 0U);
    EXPECT_GE(snapshot.cpu.usagePercent, 0.0);
    EXPECT_LE(snapshot.cpu.usagePercent, 100.0);
    EXPECT_GT(snapshot.memory.totalBytes, 0U);
    ASSERT_FALSE(snapshot.disks.empty());
    EXPECT_EQ(snapshot.disks.front().mountPoint, "/");
    EXPECT_FALSE(snapshot.system.hostname.empty());
    EXPECT_EQ(snapshot.system.operatingSystem, "Linux");
    EXPECT_FALSE(snapshot.system.architecture.empty());
    EXPECT_GT(snapshot.system.uptimeSeconds, 0U);
}
#endif