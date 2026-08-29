#pragma once

#include "gpio/gpio_provider.hpp"
#include "gpio_model.hpp"
#include "system/system_reader.hpp"

#include <memory>

namespace system_monitor {

class GpiodGpioProvider final : public GpioProvider {
public:
    explicit GpiodGpioProvider(std::shared_ptr<SystemReader> reader);

    [[nodiscard]] std::vector<GpioChipInfo> chips() override;

private:
    std::shared_ptr<SystemReader> reader_;
};

} // namespace system_monitor