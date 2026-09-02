#include "spi/spidev_spi_provider.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

namespace {

class Reader final : public system_monitor::SystemReader {
public:
    system_monitor::CommandResult result;
    std::string command;

    std::string readFile(std::string_view) override {
        throw std::runtime_error("unexpected");
    }

    system_monitor::CommandResult execute(std::string_view value) override {
        command = std::string(value);
        return result;
    }
};

} // namespace

TEST(SpidevSpiProviderTests, ParsesDeviceNodes) {
    auto reader = std::make_shared<Reader>();
    reader->result = {
        0, "spidev0.0\nspidev0.1\nspidev10.2\n", {}
    };

    system_monitor::SpidevSpiProvider provider(reader);
    auto devices = provider.devices();

    ASSERT_EQ(devices.size(), 3U);
    EXPECT_EQ(devices[0].bus, 0U);
    EXPECT_EQ(devices[0].chipSelect, 0U);
    EXPECT_EQ(devices[0].path, "/dev/spidev0.0");
    EXPECT_EQ(devices[2].bus, 10U);
}

TEST(SpidevSpiProviderTests, EmptyInventoryIsValid) {
    auto reader = std::make_shared<Reader>();
    reader->result = {
        0, "", {}
    };

    system_monitor::SpidevSpiProvider provider(reader);

    EXPECT_TRUE(provider.devices().empty());
    EXPECT_NE(reader->command.find("; true"), std::string::npos);
}

TEST(SpidevSpiProviderTests, RejectsNullReader) {
    EXPECT_THROW(
        system_monitor::SpidevSpiProvider(nullptr),
        std::invalid_argument);
}