#pragma once

#include "i2c/i2c_provider.hpp"
#include "system/system_reader.hpp"

#include <memory>
#include <unordered_set>
#include <vector>

namespace system_monitor {

class I2cToolsProvider final : public I2cProvider {
public:
    explicit I2cToolsProvider(
        std::shared_ptr<SystemReader> reader,
        std::vector<std::uint32_t> scanBuses = {});

    [[nodiscard]] std::vector<I2cBusInfo> buses() override;
    [[nodiscard]] std::optional<std::vector<I2cDeviceInfo>> devices(
        std::uint32_t bus) override;

private:
    std::shared_ptr<SystemReader> reader_;
    std::unordered_set<std::uint32_t> scanBuses_;
};

} // namespace system_monitor