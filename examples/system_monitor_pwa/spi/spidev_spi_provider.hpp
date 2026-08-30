#pragma once

#include "spi/spi_provider.hpp"
#include "system/system_reader.hpp"

#include <memory>

namespace system_monitor {

class SpidevSpiProvider final : public SpiProvider {
public:
    explicit SpidevSpiProvider(std::shared_ptr<SystemReader> reader);
    [[nodiscard]] std::vector<SpiDeviceInfo> devices() override;

private:
    std::shared_ptr<SystemReader> reader_;
};

} // namespace system_monitor