#include "i2c/i2c_tools_provider.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

class FakeSystemReader final : public system_monitor::SystemReader {
public:
    std::unordered_map<std::string, system_monitor::CommandResult> results;
    std::vector<std::string> commands;

    std::string readFile(std::string_view) override {
        throw std::runtime_error("unexpected readFile");
    }

    system_monitor::CommandResult execute(std::string_view command) override {
        commands.emplace_back(command);
        const auto found = results.find(std::string(command));
        if (found == results.end()) {
            throw std::runtime_error("unexpected command: " + std::string(command));
        }
        return found->second;
    }
};

} // namespace

TEST(I2cToolsProviderTests, ParsesRaspberryPiBusInventory) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin i2cdetect -l"] = {
        0,
        "i2c-1\tunknown   \tbcm2835 (i2c@7e804000)         \tN/A\n"
        "i2c-20\tunknown   \tfef04500.i2c                    \tN/A\n"
        "i2c-21\tunknown   \tfef09500.i2c                    \tN/A\n",
        {}
    };

    system_monitor::I2cToolsProvider provider(reader);
    const auto buses = provider.buses();

    ASSERT_EQ(buses.size(), 3U);
    EXPECT_EQ(buses[0].number, 1U);
    EXPECT_EQ(buses[0].name, "i2c-1");
    EXPECT_EQ(buses[0].type, "unknown");
    EXPECT_EQ(buses[0].description, "bcm2835 (i2c@7e804000)");
    EXPECT_EQ(buses[0].algorithm, "N/A");
    EXPECT_EQ(buses[2].number, 21U);
}

TEST(I2cToolsProviderTests, ParsesDetectedAddressesIncludingKernelClaimed) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin i2cdetect -y 1"] = {
        0,
        "     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n"
        "00:                         -- -- -- -- -- -- -- --\n"
        "10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n"
        "20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n"
        "30: -- -- -- -- -- -- -- -- -- -- -- -- 3c -- -- --\n"
        "40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n"
        "50: UU -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n"
        "60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n"
        "70: -- -- -- -- -- -- -- --\n",
        {}
    };

    system_monitor::I2cToolsProvider provider(reader, {1});
    const auto devices = provider.devices(1);

    ASSERT_TRUE(devices.has_value());
    ASSERT_EQ(devices->size(), 2U);
    EXPECT_EQ((*devices)[0].address, 0x3c);
    EXPECT_FALSE((*devices)[0].claimedByKernel);
    EXPECT_EQ((*devices)[1].address, 0x50);
    EXPECT_TRUE((*devices)[1].claimedByKernel);
}

TEST(I2cToolsProviderTests, IgnoresMalformedBusLines) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin i2cdetect -l"] = {
        0,
        "warning\n"
        "i2c-X\tunknown\tbad\tN/A\n"
        "i2c-1\tunknown\tgood\tN/A\n",
        {}
    };

    system_monitor::I2cToolsProvider provider(reader);
    const auto buses = provider.buses();

    ASSERT_EQ(buses.size(), 1U);
    EXPECT_EQ(buses.front().number, 1U);
}

TEST(I2cToolsProviderTests, ReportsMissingI2cTools) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin i2cdetect -l"] = {
        127, {}, "i2cdetect: command not found"
    };

    system_monitor::I2cToolsProvider provider(reader);
    EXPECT_THROW(static_cast<void>(provider.buses()), std::runtime_error);
}

TEST(I2cToolsProviderTests, ReportsBusPermissionFailure) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin i2cdetect -y 1"] = {
        1, {}, "Could not open file `/dev/i2c-1': Permission denied"
    };

    system_monitor::I2cToolsProvider provider(reader, {1});
    EXPECT_THROW(static_cast<void>(provider.devices(1)), std::runtime_error);
}

TEST(I2cToolsProviderTests, RejectsNullReader) {
    EXPECT_THROW(
        system_monitor::I2cToolsProvider(nullptr),
        std::invalid_argument);
}


TEST(I2cToolsProviderTests, DoesNotProbeUnconfiguredBus) {
    auto reader = std::make_shared<FakeSystemReader>();
    system_monitor::I2cToolsProvider provider(reader);

    const auto devices = provider.devices(1);

    EXPECT_FALSE(devices.has_value());
    EXPECT_TRUE(reader->commands.empty());
}