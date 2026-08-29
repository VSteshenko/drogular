#pragma once

#include "services/gpio_service.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace system_monitor {

enum class I2cGpioRole {
    Sda,
    Scl
};

struct I2cGpioPin {
    I2cGpioRole role{I2cGpioRole::Sda};
    std::string chip;
    std::uint32_t offset{0};
    std::string name;
    std::string function;
};

class I2cGpioCorrelator {
public:
    [[nodiscard]] static std::vector<I2cGpioPin> pinsForBus(
        std::uint32_t busNumber,
        const GpioSnapshot& gpioSnapshot);
};

} // namespace system_monitor