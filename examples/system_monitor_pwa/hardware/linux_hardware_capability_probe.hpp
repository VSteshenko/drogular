#pragma once

#include "system/system_reader.hpp"

namespace system_monitor {

enum class LinuxHardwareCapability {
    Gpio,
    I2c,
    Spi,
    Uart
};

[[nodiscard]] bool hasLinuxHardwareCapability(
    SystemReader& reader,
    LinuxHardwareCapability capability);

} // namespace system_monitor