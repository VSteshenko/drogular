#pragma once

#include "gpio/gpio_model.hpp"

#include <string_view>
#include <vector>

namespace system_monitor {

class GpioProvider {
public:
    virtual ~GpioProvider() = default;

    [[nodiscard]] virtual std::vector<GpioChipInfo> chips() = 0;
    [[nodiscard]] virtual std::vector<GpioLineInfo> lines(std::string_view chip) = 0;
};

} // namespace system_monitor