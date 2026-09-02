#include "uart/linux_uart_provider.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

namespace {

class Reader final : public system_monitor::SystemReader {
public:
    system_monitor::CommandResult result;
    std::string command;
    std::string readFile(std::string_view) override { throw std::runtime_error("unexpected"); }
    system_monitor::CommandResult execute(std::string_view value) override { command = std::string(value); return result; }
};

} // namespace

TEST(LinuxUartProviderTests, ParsesDevicesAndSerialAliases) {
    auto reader = std::make_shared<Reader>();
    reader->result = {0,
        "device\t/dev/ttyAMA0\n"
        "device\t/dev/ttyS0\n"
        "alias\t/dev/serial0\t/dev/ttyS0\n"
        "alias\t/dev/serial1\t/dev/ttyAMA0\n", {}};
    system_monitor::LinuxUartProvider provider(reader);

    const auto devices = provider.devices();
    ASSERT_EQ(devices.size(), 2U);
    EXPECT_EQ(devices[0].path, "/dev/ttyAMA0");
    ASSERT_EQ(devices[0].aliases.size(), 1U);
    EXPECT_EQ(devices[0].aliases[0], "/dev/serial1");
    EXPECT_EQ(devices[1].path, "/dev/ttyS0");
    EXPECT_EQ(devices[1].aliases[0], "/dev/serial0");
    EXPECT_NE(reader->command.find("readlink -f"), std::string::npos);
}

TEST(LinuxUartProviderTests, AddsAliasTargetWhenDeviceGlobDidNotListIt) {
    auto reader = std::make_shared<Reader>();
    reader->result = {
        0,
        "alias\t/dev/serial0\t/dev/ttyUSB0\n",
        {}
    };
    system_monitor::LinuxUartProvider provider(reader);

    const auto devices = provider.devices();
    ASSERT_EQ(devices.size(), 1U);
    EXPECT_EQ(devices[0].name, "ttyUSB0");
    EXPECT_EQ(devices[0].aliases[0], "/dev/serial0");
}

TEST(LinuxUartProviderTests, EmptyInventoryIsValid) {
    auto reader = std::make_shared<Reader>();
    reader->result = { 0, "", {} };
    system_monitor::LinuxUartProvider provider(reader);
    EXPECT_TRUE(provider.devices().empty());
    EXPECT_NE(reader->command.find("; true"), std::string::npos);
}

TEST(LinuxUartProviderTests, RejectsNullReader) {
    EXPECT_THROW(
        system_monitor::LinuxUartProvider(nullptr),
        std::invalid_argument);
}