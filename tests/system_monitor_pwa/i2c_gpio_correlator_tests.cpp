#include "hardware/i2c_gpio_correlator.hpp"

#include <gtest/gtest.h>

#include <utility>

namespace {

system_monitor::GpioLineInfo line(
    std::uint32_t offset,
    std::string name,
    std::string function
) {
    return system_monitor::GpioLineInfo{
        .offset = offset,
        .name = std::move(name),
        .function = std::move(function),
        .alternateFunction = true
    };
}

system_monitor::BoardGpioMetadata raspberryPi4Metadata() {
    system_monitor::SystemSnapshot snapshot;
    snapshot.raspberryPi = system_monitor::RaspberryPiInfo{
        .model = "Raspberry Pi 4 Model B Rev 1.4"
    };
    return system_monitor::BoardGpioMetadata::fromSystemSnapshot(snapshot);
}

} // namespace

TEST(I2cGpioCorrelatorTests, CorrelatesBusWithMatchingSdaAndSclFunctions) {
    system_monitor::GpioSnapshot gpio;
    gpio.chips.push_back({
        .chip = {.name = "gpiochip0", .label = "pinctrl-bcm2711", .lineCount = 58},
        .lines = {
            line(2, "GPIO2", "SDA1"),
            line(3, "GPIO3", "SCL1"),
            line(4, "GPIO4", "GPCLK0")
        }
    });

    const auto pins = system_monitor::I2cGpioCorrelator::pinsForBus(
        1, gpio, raspberryPi4Metadata());

    ASSERT_EQ(pins.size(), 2U);
    EXPECT_EQ(pins[0].role, system_monitor::I2cGpioRole::Sda);
    EXPECT_EQ(pins[0].chip, "gpiochip0");
    EXPECT_EQ(pins[0].offset, 2U);
    EXPECT_EQ(pins[0].name, "GPIO2");
    EXPECT_EQ(pins[0].function, "SDA1");
    EXPECT_EQ(pins[1].role, system_monitor::I2cGpioRole::Scl);
    EXPECT_EQ(pins[1].offset, 3U);
    EXPECT_EQ(pins[1].function, "SCL1");
    EXPECT_EQ(pins[0].exposure, system_monitor::GpioExposure::Header);
    ASSERT_TRUE(pins[0].physicalHeaderPin.has_value());
    EXPECT_EQ(*pins[0].physicalHeaderPin, 3U);
    ASSERT_TRUE(pins[1].physicalHeaderPin.has_value());
    EXPECT_EQ(*pins[1].physicalHeaderPin, 5U);
}

TEST(I2cGpioCorrelatorTests, IgnoresFunctionsFromOtherBusNumbers) {
    system_monitor::GpioSnapshot gpio;
    gpio.chips.push_back({
        .chip = {.name = "gpiochip0", .lineCount = 58},
        .lines = {
            line(0, "ID_SDA", "SDA0"),
            line(1, "ID_SCL", "SCL0"),
            line(2, "GPIO2", "SDA1"),
            line(3, "GPIO3", "SCL1")
        }
    });

    const auto pins = system_monitor::I2cGpioCorrelator::pinsForBus(20, gpio);
    EXPECT_TRUE(pins.empty());
}