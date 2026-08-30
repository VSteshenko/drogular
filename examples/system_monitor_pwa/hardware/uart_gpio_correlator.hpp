#pragma once

#include "hardware/board_gpio_metadata.hpp"
#include "services/gpio_service.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace system_monitor {

struct UartGpioPin {
    std::string role;
    std::string chip;
    std::uint32_t offset{0};
    std::string name;
    std::string function;
    std::string consumer;
    GpioExposure exposure{GpioExposure::Unknown};
    std::optional<std::uint32_t> physicalHeaderPin;
};

struct UartGpioGroup {
    std::uint32_t controller{0};
    GpioExposure exposure{GpioExposure::Unknown};
    std::vector<UartGpioPin> pins;
};

class UartGpioCorrelator {
public:
    [[nodiscard]] static std::vector<UartGpioGroup> groups(
        const GpioSnapshot& gpioSnapshot,
        const BoardGpioMetadata& boardMetadata = {});
};

} // namespace system_monitor