#pragma once

#include "gpio/gpio_provider.hpp"
#include "system/system_reader.hpp"

#include <memory>
#include <string_view>

namespace system_monitor {

class GpiodGpioProvider final : public GpioProvider {
public:
    explicit GpiodGpioProvider(std::shared_ptr<SystemReader> reader);

    [[nodiscard]] std::vector<GpioChipInfo> chips() override;
    [[nodiscard]] std::vector<GpioLineInfo> lines(std::string_view chip) override;

private:
    std::shared_ptr<SystemReader> reader_;
};

} // namespace system_monitor