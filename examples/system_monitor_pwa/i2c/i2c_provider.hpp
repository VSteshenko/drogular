#pragma once

#include "i2c/i2c_model.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace system_monitor {

class I2cProvider {
public:
    virtual ~I2cProvider() = default;

    [[nodiscard]] virtual std::vector<I2cBusInfo> buses() = 0;
    [[nodiscard]] virtual std::optional<std::vector<I2cDeviceInfo>> devices(
        std::uint32_t bus) = 0;
};

} // namespace system_monitor