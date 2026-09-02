#include "hardware/linux_hardware_capability_probe.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

class Reader final : public system_monitor::SystemReader {
public:
    system_monitor::CommandResult result;
    std::string command;

    std::string readFile(std::string_view) override { return {}; }
    system_monitor::CommandResult execute(std::string_view value) override {
        command = std::string(value);
        return result;
    }
};

} // namespace

TEST(LinuxHardwareCapabilityProbeTests, ReportsCapabilityWhenProbeSucceeds) {
    Reader reader;
    reader.result = {0, {}, {}};

    EXPECT_TRUE(system_monitor::hasLinuxHardwareCapability(
        reader, system_monitor::LinuxHardwareCapability::Gpio));
    EXPECT_NE(reader.command.find("command -v gpiodetect"), std::string::npos);
    EXPECT_NE(reader.command.find("/dev/gpiochip"), std::string::npos);
    EXPECT_NE(reader.command.find("/usr/sbin"), std::string::npos);
}

TEST(LinuxHardwareCapabilityProbeTests, MissingCapabilityIsNormal) {
    Reader reader;
    reader.result = {1, {}, {}};

    EXPECT_FALSE(system_monitor::hasLinuxHardwareCapability(
        reader, system_monitor::LinuxHardwareCapability::Spi));
    EXPECT_NE(reader.command.find("/dev/spidev"), std::string::npos);
}

TEST(LinuxHardwareCapabilityProbeTests, I2cRequiresToolAndDeviceNode) {
    Reader reader;
    reader.result = {1, {}, {}};

    EXPECT_FALSE(system_monitor::hasLinuxHardwareCapability(
        reader, system_monitor::LinuxHardwareCapability::I2c));
    EXPECT_NE(reader.command.find("command -v i2cdetect"), std::string::npos);
    EXPECT_NE(reader.command.find("/dev/i2c-"), std::string::npos);
}

TEST(LinuxHardwareCapabilityProbeTests, UartAcceptsCharacterDevicesOrSerialAliases) {
    Reader reader;
    reader.result = {0, {}, {}};

    EXPECT_TRUE(system_monitor::hasLinuxHardwareCapability(
        reader, system_monitor::LinuxHardwareCapability::Uart));
    EXPECT_NE(reader.command.find("/dev/ttyAMA"), std::string::npos);
    EXPECT_NE(reader.command.find("/dev/serial"), std::string::npos);
}