#include "ui/system_formatters.hpp"

#include <gtest/gtest.h>

TEST(SystemMonitorFormattersTests, FormatsByteUnits) {
    EXPECT_EQ(system_monitor::ui::formatBytes(512), "512 B");
    EXPECT_EQ(system_monitor::ui::formatBytes(1024), "1.00 KiB");
    EXPECT_EQ(system_monitor::ui::formatBytes(10ULL * 1024 * 1024), "10.0 MiB");
}

TEST(SystemMonitorFormattersTests, FormatsDuration) {
    EXPECT_EQ(system_monitor::ui::formatDuration(59), "0m");
    EXPECT_EQ(system_monitor::ui::formatDuration(3660), "1h 1m");
    EXPECT_EQ(system_monitor::ui::formatDuration(90060), "1d 1h 1m");
}

TEST(SystemMonitorFormattersTests, FormatsAndClampsPercent) {
    EXPECT_EQ(system_monitor::ui::formatPercent(17.42), "17.4%");
    EXPECT_EQ(system_monitor::ui::formatPercent(-1.0), "0.0%");
    EXPECT_EQ(system_monitor::ui::formatPercent(120.0), "100.0%");
}

TEST(SystemMonitorFormattersTests, FormatsTemperature) {
    EXPECT_EQ(system_monitor::ui::formatTemperature(48.725), "48.7 °C");
}

TEST(SystemMonitorFormattersTests, CalculatesPercentSafely) {
    EXPECT_DOUBLE_EQ(system_monitor::ui::percentOf(50, 200), 25.0);
    EXPECT_DOUBLE_EQ(system_monitor::ui::percentOf(1, 0), 0.0);
    EXPECT_DOUBLE_EQ(system_monitor::ui::percentOf(200, 100), 100.0);
}