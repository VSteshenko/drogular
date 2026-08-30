#pragma once

#include "system/system_reader.hpp"
#include "uart_provider.hpp"

#include <memory>

namespace system_monitor {

class LinuxUartProvider final : public UartProvider {
public:
    explicit LinuxUartProvider(std::shared_ptr<SystemReader> reader);
    [[nodiscard]] std::vector<UartDeviceInfo> devices() override;

private:
    std::shared_ptr<SystemReader> reader_;
};

} // namespace system_monitor