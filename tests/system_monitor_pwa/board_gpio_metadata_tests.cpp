#include "hardware/board_gpio_metadata.hpp"

#include <gtest/gtest.h>

TEST(BoardGpioMetadataTests, MapsRaspberryPiHeaderGpioToPhysicalPins) {
    system_monitor::SystemSnapshot snapshot;
    snapshot.raspberryPi = system_monitor::RaspberryPiInfo{
        .model = "Raspberry Pi 4 Model B Rev 1.4"
    };

    const auto metadata = system_monitor::BoardGpioMetadata::fromSystemSnapshot(snapshot);
    ASSERT_TRUE(metadata.available());

    const auto tx = metadata.line("gpiochip0", 14);
    EXPECT_EQ(tx.exposure, system_monitor::GpioExposure::Header);
    ASSERT_TRUE(tx.physicalHeaderPin.has_value());
    EXPECT_EQ(*tx.physicalHeaderPin, 8U);

    const auto rx = metadata.line("gpiochip0", 15);
    EXPECT_EQ(rx.exposure, system_monitor::GpioExposure::Header);
    ASSERT_TRUE(rx.physicalHeaderPin.has_value());
    EXPECT_EQ(*rx.physicalHeaderPin, 10U);
}

TEST(BoardGpioMetadataTests, MarksNonHeaderPrimaryChipLinesInternal) {
    system_monitor::SystemSnapshot snapshot;
    snapshot.raspberryPi = system_monitor::RaspberryPiInfo{
        .model = "Raspberry Pi 4 Model B Rev 1.4"
    };

    const auto metadata = system_monitor::BoardGpioMetadata::fromSystemSnapshot(snapshot);
    const auto line = metadata.line("gpiochip0", 32);
    EXPECT_EQ(line.exposure, system_monitor::GpioExposure::Internal);
    EXPECT_FALSE(line.physicalHeaderPin.has_value());
}

TEST(BoardGpioMetadataTests, MarksExpansionChipSignalsOnBoard) {
    system_monitor::SystemSnapshot snapshot;
    snapshot.raspberryPi = system_monitor::RaspberryPiInfo{
        .model = "Raspberry Pi 4 Model B Rev 1.4"
    };

    const auto metadata = system_monitor::BoardGpioMetadata::fromSystemSnapshot(snapshot);
    const auto line = metadata.line("gpiochip1", 2);
    EXPECT_EQ(line.exposure, system_monitor::GpioExposure::OnBoard);
}

TEST(BoardGpioMetadataTests, KeepsUnknownBoardsUnclassified) {
    system_monitor::SystemSnapshot snapshot;
    const auto metadata = system_monitor::BoardGpioMetadata::fromSystemSnapshot(snapshot);
    EXPECT_FALSE(metadata.available());
    EXPECT_EQ(metadata.line("gpiochip0", 14).exposure,
              system_monitor::GpioExposure::Unknown);
}

TEST(BoardGpioMetadataTests, DoesNotApplyHeaderMapToComputeModules) {
    system_monitor::SystemSnapshot snapshot;
    snapshot.raspberryPi = system_monitor::RaspberryPiInfo{
        .model = "Raspberry Pi Compute Module 4 Rev 1.0"
    };

    const auto metadata = system_monitor::BoardGpioMetadata::fromSystemSnapshot(snapshot);
    EXPECT_FALSE(metadata.available());
}