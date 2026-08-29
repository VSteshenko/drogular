#pragma once

#include "gpio/gpio_model.hpp"

#include <vector>

namespace system_monitor {

class GpioProvider {
public:
    virtual ~GpioProvider() = default;

    [[nodiscard]] virtual std::vector<GpioChipInfo> chips() = 0;
};

} // namespace system_monitor