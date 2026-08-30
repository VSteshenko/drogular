#pragma once

#include "hardware/board_gpio_metadata.hpp"
#include "services/gpio_service.hpp"

#include <cstdint>
#include <optional>
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
    GpioExposure exposure{GpioExposure::Unknown};
    std::optional<std::uint32_t> physicalHeaderPin;
};

class I2cGpioCorrelator {
public:
    [[nodiscard]] static std::vector<I2cGpioPin> pinsForBus(
        std::uint32_t busNumber,
        const GpioSnapshot& gpioSnapshot,
        const BoardGpioMetadata& boardMetadata = {});
};

} // namespace system_monitor