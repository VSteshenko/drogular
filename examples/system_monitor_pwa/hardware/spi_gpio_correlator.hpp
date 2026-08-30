#pragma once

#include "services/gpio_service.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace system_monitor {

struct SpiGpioPin {
    std::string role;
    std::string chip;
    std::uint32_t offset{0};
    std::string name;
    std::string function;
    std::string consumer;
};

class SpiGpioCorrelator {
public:
    [[nodiscard]] static std::vector<SpiGpioPin> pinsForBus(
        std::uint32_t busNumber,
        const GpioSnapshot& gpioSnapshot);
};

} // namespace system_monitor