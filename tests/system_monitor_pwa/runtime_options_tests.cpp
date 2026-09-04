#include "configuration/runtime_options.hpp"

#include <gtest/gtest.h>

#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace {

system_monitor::EnvironmentLookup environmentWith(
    std::map<std::string, std::string> values) {
    return [values = std::move(values)](std::string_view name)
        -> std::optional<std::string> {
        const auto found = values.find(std::string(name));
        if (found == values.end()) {
            return std::nullopt;
        }
        return found->second;
    };
}

} // namespace

TEST(RuntimeOptionsTests, DefaultsToLocalTarget) {
    const char* argv[] = {"system_monitor_pwa"};

    const auto options = system_monitor::parseRuntimeOptions(
        1, argv, environmentWith({}));

    EXPECT_EQ(options.target.type, system_monitor::MonitorTargetType::Local);
    EXPECT_TRUE(options.i2cScanBuses.empty());
    EXPECT_FALSE(options.showHelp);
}

TEST(RuntimeOptionsTests, ReadsSshConfigurationFromEnvironment) {
    const char* argv[] = {"system_monitor_pwa"};
    const auto environment = environmentWith({
        {"SYSTEM_MONITOR_TARGET", "ssh"},
        {"SYSTEM_MONITOR_SSH_HOST", "pi.local"},
        {"SYSTEM_MONITOR_SSH_PORT", "2222"},
        {"SYSTEM_MONITOR_SSH_USER", "monitor"},
        {"SYSTEM_MONITOR_SSH_IDENTITY_FILE", "/keys/pi"},
        {"SYSTEM_MONITOR_SSH_KNOWN_HOSTS_FILE", "/keys/known_hosts"},
        {"SYSTEM_MONITOR_I2C_SCAN_BUSES", "1, 4"},
    });

    const auto options = system_monitor::parseRuntimeOptions(1, argv, environment);

    EXPECT_EQ(options.target.type, system_monitor::MonitorTargetType::Ssh);
    EXPECT_EQ(options.target.ssh.host, "pi.local");
    EXPECT_EQ(options.target.ssh.port, 2222);
    EXPECT_EQ(options.target.ssh.user, "monitor");
    EXPECT_EQ(options.target.ssh.identityFile, "/keys/pi");
    EXPECT_EQ(options.target.ssh.knownHostsFile, "/keys/known_hosts");
    ASSERT_EQ(options.i2cScanBuses.size(), 2U);
    EXPECT_EQ(options.i2cScanBuses[0], 1U);
    EXPECT_EQ(options.i2cScanBuses[1], 4U);
}

TEST(RuntimeOptionsTests, CommandLineOverridesEnvironment) {
    const char* argv[] = {
        "system_monitor_pwa",
        "--ssh-host", "cli.local",
        "--ssh-port=2200",
        "--ssh-user", "cli-user",
        "--i2c-scan-buses=2,3",
    };
    const auto environment = environmentWith({
        {"SYSTEM_MONITOR_TARGET", "ssh"},
        {"SYSTEM_MONITOR_SSH_HOST", "env.local"},
        {"SYSTEM_MONITOR_SSH_PORT", "22"},
        {"SYSTEM_MONITOR_SSH_USER", "env-user"},
        {"SYSTEM_MONITOR_SSH_IDENTITY_FILE", "/keys/pi"},
        {"SYSTEM_MONITOR_SSH_KNOWN_HOSTS_FILE", "/keys/known_hosts"},
        {"SYSTEM_MONITOR_I2C_SCAN_BUSES", "1"},
    });

    const auto options = system_monitor::parseRuntimeOptions(
        static_cast<int>(std::size(argv)), argv, environment);

    EXPECT_EQ(options.target.type, system_monitor::MonitorTargetType::Ssh);
    EXPECT_EQ(options.target.ssh.host, "cli.local");
    EXPECT_EQ(options.target.ssh.port, 2200);
    EXPECT_EQ(options.target.ssh.user, "cli-user");
    ASSERT_EQ(options.i2cScanBuses.size(), 2U);
    EXPECT_EQ(options.i2cScanBuses[0], 2U);
    EXPECT_EQ(options.i2cScanBuses[1], 3U);
}

TEST(RuntimeOptionsTests, CommandLineCanOverrideEnvironmentTargetToLocal) {
    const char* argv[] = {"system_monitor_pwa", "--target", "local"};
    const auto environment = environmentWith({
        {"SYSTEM_MONITOR_TARGET", "ssh"},
        {"SYSTEM_MONITOR_SSH_PORT", "invalid"},
    });

    const auto options = system_monitor::parseRuntimeOptions(3, argv, environment);

    EXPECT_EQ(options.target.type, system_monitor::MonitorTargetType::Local);
}

TEST(RuntimeOptionsTests, RejectsMissingSshFieldsAfterOverridesAreApplied) {
    const char* argv[] = {"system_monitor_pwa", "--target=ssh"};

    EXPECT_THROW(
        (void)system_monitor::parseRuntimeOptions(2, argv, environmentWith({})),
        std::invalid_argument);
}

TEST(RuntimeOptionsTests, RejectsInvalidPort) {
    const char* argv[] = {
        "system_monitor_pwa",
        "--target=ssh",
        "--ssh-host=pi.local",
        "--ssh-port=70000",
        "--ssh-user=monitor",
        "--ssh-identity-file=/keys/pi",
        "--ssh-known-hosts-file=/keys/known_hosts",
    };

    EXPECT_THROW(
        (void)system_monitor::parseRuntimeOptions(
            static_cast<int>(std::size(argv)), argv, environmentWith({})),
        std::invalid_argument);
}

TEST(RuntimeOptionsTests, RejectsUnknownOption) {
    const char* argv[] = {"system_monitor_pwa", "--unknown=value"};

    EXPECT_THROW(
        (void)system_monitor::parseRuntimeOptions(2, argv, environmentWith({})),
        std::invalid_argument);
}

TEST(RuntimeOptionsTests, HelpDoesNotRequireValidEnvironment) {
    const char* argv[] = {"system_monitor_pwa", "--help"};
    const auto environment = environmentWith({
        {"SYSTEM_MONITOR_TARGET", "ssh"},
        {"SYSTEM_MONITOR_SSH_PORT", "invalid"},
    });

    const auto options = system_monitor::parseRuntimeOptions(2, argv, environment);

    EXPECT_TRUE(options.showHelp);
}

TEST(RuntimeOptionsTests, HelpDocumentsOverridePrecedence) {
    const auto help = system_monitor::runtimeOptionsHelp("system_monitor_pwa");

    EXPECT_NE(help.find("Command-line options override"), std::string::npos);
    EXPECT_NE(help.find("--target <local|ssh>"), std::string::npos);
    EXPECT_NE(help.find("--ssh-known-hosts-file <path>"), std::string::npos);
    EXPECT_NE(help.find("--i2c-scan-buses <list>"), std::string::npos);
    EXPECT_NE(help.find("SYSTEM_MONITOR_TARGET"), std::string::npos);
}