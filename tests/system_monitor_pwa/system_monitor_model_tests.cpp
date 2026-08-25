#include "system/monitor_target.hpp"
#include "system/system_metrics_provider.hpp"
#include "system/system_reader.hpp"
#include "system/system_snapshot.hpp"

#include <gtest/gtest.h>

#include <utility>

using namespace system_monitor;

TEST(SystemMonitorModelTests, LocalTargetIsDefault) {
    const auto target = MonitorTarget::local();

    EXPECT_EQ(target.type, MonitorTargetType::Local);
    EXPECT_EQ(target.ssh.port, 22);
}

TEST(SystemMonitorModelTests, RemoteTargetKeepsSshConfiguration) {
    SshTargetOptions options;
    options.host = "raspberrypi.local";
    options.port = 2222;
    options.user = "monitor";
    options.identityFile = "/keys/id_ed25519";
    options.knownHostsFile = "/keys/known_hosts";

    const auto target = MonitorTarget::remote(std::move(options));

    EXPECT_EQ(target.type, MonitorTargetType::Ssh);
    EXPECT_EQ(target.ssh.host, "raspberrypi.local");
    EXPECT_EQ(target.ssh.port, 2222);
    EXPECT_EQ(target.ssh.user, "monitor");
    EXPECT_EQ(target.ssh.identityFile, "/keys/id_ed25519");
    EXPECT_EQ(target.ssh.knownHostsFile, "/keys/known_hosts");
}

TEST(SystemMonitorModelTests, SnapshotCanDescribeRaspberryPi) {
    SystemSnapshot snapshot;
    snapshot.system.hostname = "raspberrypi";
    snapshot.system.architecture = "aarch64";
    snapshot.cpu.logicalCores = 4;
    snapshot.memory.totalBytes = 4ULL * 1024ULL * 1024ULL * 1024ULL;
    snapshot.raspberryPi = RaspberryPiInfo{
        .model = "Raspberry Pi 5 Model B",
        .revision = "1.0",
        .serial = "0123456789abcdef",
        .temperatureCelsius = 52.5
    };

    ASSERT_TRUE(snapshot.raspberryPi.has_value());
    EXPECT_EQ(snapshot.raspberryPi->model, "Raspberry Pi 5 Model B");
    EXPECT_DOUBLE_EQ(*snapshot.raspberryPi->temperatureCelsius, 52.5);
}

TEST(SystemMonitorModelTests, CommandResultUsesExitCodeForSuccess) {
    const CommandResult success{0, "ok", ""};
    const CommandResult failure{1, "", "failed"};

    EXPECT_TRUE(success.succeeded());
    EXPECT_FALSE(failure.succeeded());
}