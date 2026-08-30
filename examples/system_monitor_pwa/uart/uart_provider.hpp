#pragma once

#include "uart_model.hpp"

#include <vector>

namespace system_monitor {

class UartProvider {
public:
    virtual ~UartProvider() = default;
    [[nodiscard]] virtual std::vector<UartDeviceInfo> devices() = 0;
};

} // namespace system_monitor