#include "hardware/board_gpio_metadata.hpp"
#include "hardware/uart_gpio_correlator.hpp"

#include <gtest/gtest.h>

TEST(UartGpioCorrelatorTests, GroupsActiveUartPinmuxFunctionsByController) {
    system_monitor::GpioSnapshot snapshot;
    system_monitor::GpioChipSnapshot chip;
    chip.chip = { "gpiochip0", "pinctrl", 58 };
    chip.lines = {
        { .offset = 14, .name = "GPIO14", .function = "TXD1", .alternateFunction = true },
        { .offset = 15, .name = "GPIO15", .function = "RXD1", .alternateFunction = true },
        { .offset = 30, .name = "CTS0", .function = "CTS0", .alternateFunction = true },
        { .offset = 31, .name = "RTS0", .function = "RTS0", .alternateFunction = true },
        { .offset = 2, .name = "GPIO2", .function = "SDA1", .alternateFunction = true }
    };
    snapshot.chips.push_back(chip);

    const auto groups = system_monitor::UartGpioCorrelator::groups(snapshot);
    ASSERT_EQ(groups.size(), 2U);
    EXPECT_EQ(groups[0].controller, 0U);
    ASSERT_EQ(groups[0].pins.size(), 2U);
    EXPECT_EQ(groups[0].pins[0].role, "cts");
    EXPECT_EQ(groups[0].pins[1].role, "rts");
    EXPECT_EQ(groups[1].controller, 1U);
    ASSERT_EQ(groups[1].pins.size(), 2U);
    EXPECT_EQ(groups[1].pins[0].role, "txd");
    EXPECT_EQ(groups[1].pins[1].role, "rxd");
}

TEST(UartGpioCorrelatorTests, IgnoresNonUartFunctions) {
    system_monitor::GpioSnapshot snapshot;
    system_monitor::GpioChipSnapshot chip;
    chip.chip = { "gpiochip0", "pinctrl", 58 };
    chip.lines={
        { .offset = 9, .name = "GPIO9", .function = "SPI0_MISO", .alternateFunction = true }
    };
    snapshot.chips.push_back(chip);

    EXPECT_TRUE(system_monitor::UartGpioCorrelator::groups(snapshot).empty());
}

TEST(UartGpioCorrelatorTests, AppliesBoardExposureToGroupsAndPins) {
    system_monitor::SystemSnapshot systemSnapshot;
    systemSnapshot.raspberryPi = system_monitor::RaspberryPiInfo{
        .model = "Raspberry Pi 4 Model B Rev 1.4"
    };
    const auto metadata = system_monitor::BoardGpioMetadata::fromSystemSnapshot(systemSnapshot);

    system_monitor::GpioSnapshot snapshot;
    system_monitor::GpioChipSnapshot chip;
    chip.chip = { "gpiochip0", "pinctrl-bcm2711", 58 };
    chip.lines = {
        { .offset = 14, .name = "GPIO14", .function = "TXD1", .alternateFunction = true },
        { .offset = 15, .name = "GPIO15", .function = "RXD1", .alternateFunction = true },
        { .offset = 32, .name = "TXD0", .function = "TXD0", .alternateFunction = true },
        { .offset = 33, .name = "RXD0", .function = "RXD0", .alternateFunction = true }
    };
    snapshot.chips.push_back(chip);

    const auto groups = system_monitor::UartGpioCorrelator::groups(snapshot, metadata);
    ASSERT_EQ(groups.size(), 2U);
    EXPECT_EQ(groups[0].controller, 0U);
    EXPECT_EQ(groups[0].exposure, system_monitor::GpioExposure::Internal);
    EXPECT_EQ(groups[1].controller, 1U);
    EXPECT_EQ(groups[1].exposure, system_monitor::GpioExposure::Header);
    ASSERT_EQ(groups[1].pins.size(), 2U);
    ASSERT_TRUE(groups[1].pins[0].physicalHeaderPin.has_value());
    ASSERT_TRUE(groups[1].pins[1].physicalHeaderPin.has_value());
    EXPECT_EQ(*groups[1].pins[0].physicalHeaderPin, 8U);
    EXPECT_EQ(*groups[1].pins[1].physicalHeaderPin, 10U);
}