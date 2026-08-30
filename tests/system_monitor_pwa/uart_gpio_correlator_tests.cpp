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