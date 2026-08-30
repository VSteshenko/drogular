#pragma once

#include "services/gpio_service.hpp"

#include <cstdint>
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
};

struct UartGpioGroup {
    std::uint32_t controller{0};
    std::vector<UartGpioPin> pins;
};

class UartGpioCorrelator {
public:
    [[nodiscard]] static std::vector<UartGpioGroup> groups(const GpioSnapshot& gpioSnapshot);
};

} // namespace system_monitor