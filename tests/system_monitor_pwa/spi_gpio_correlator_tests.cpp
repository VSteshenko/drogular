#include "hardware/spi_gpio_correlator.hpp"

#include <gtest/gtest.h>

TEST(SpiGpioCorrelatorTests, MapsSpiFunctionsAndKernelConsumersForBus) {
    system_monitor::GpioSnapshot snapshot;
    system_monitor::GpioChipSnapshot chipSnapshot;
    chipSnapshot.chip = { "gpiochip0", "pinctrl", 58 };
    chipSnapshot.lines = {
        { .offset=7, .name = "GPIO7", .consumer = "spi0 CS1", .function = "output" },
        { .offset=8, .name = "GPIO8", .consumer = "spi0 CS0", .function = "output" },
        { .offset=9, .name = "GPIO9", .function = "SPI0_MISO" },
        { .offset=10, .name = "GPIO10", .function = "SPI0_MOSI" },
        { .offset=11, .name = "GPIO11", .function = "SPI0_SCLK" },
        { .offset=2, .name = "GPIO2", .function = "SDA1" }
    };
    snapshot.chips.push_back(chipSnapshot);

    auto pins = system_monitor::SpiGpioCorrelator::pinsForBus(0, snapshot);
    ASSERT_EQ(pins.size(), 5U);
    EXPECT_EQ(pins[0].role, "ce1");
    EXPECT_EQ(pins[0].consumer, "spi0 CS1");
    EXPECT_EQ(pins[1].role, "ce0");
    EXPECT_EQ(pins[1].consumer, "spi0 CS0");
    EXPECT_EQ(pins[2].role, "miso");
    EXPECT_EQ(pins[3].role, "mosi");
    EXPECT_EQ(pins[4].role, "sclk");
}

TEST(SpiGpioCorrelatorTests, IgnoresConsumersFromOtherSpiBuses) {
    system_monitor::GpioSnapshot snapshot;
    system_monitor::GpioChipSnapshot chipSnapshot;
    chipSnapshot.chip={ "gpiochip0", "pinctrl", 58 };
    chipSnapshot.lines={
        { .offset = 7, .name = "GPIO7", .consumer = "spi1 CS0", .function = "output" }
    };
    snapshot.chips.push_back(chipSnapshot);

    EXPECT_TRUE(system_monitor::SpiGpioCorrelator::pinsForBus(0, snapshot).empty());
}