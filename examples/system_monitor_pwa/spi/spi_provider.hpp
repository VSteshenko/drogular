#pragma once

#include "spi_model.hpp"

#include <vector>

namespace system_monitor {

class SpiProvider {
public:
    virtual ~SpiProvider() = default;
    [[nodiscard]] virtual std::vector<SpiDeviceInfo> devices() = 0;
};

} // namespace system_monitor