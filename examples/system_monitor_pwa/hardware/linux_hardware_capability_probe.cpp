#include "linux_hardware_capability_probe.hpp"

#include <string>
#include <string_view>

namespace system_monitor {

namespace {

constexpr std::string_view kToolPath =
    "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin; ";

constexpr std::string_view kGpioProbe =
    "command -v gpiodetect >/dev/null 2>&1 || exit 1; "
    "for p in /dev/gpiochip*; do [ -c \"$p\" ] && exit 0; done; exit 1";

constexpr std::string_view kI2cProbe =
    "command -v i2cdetect >/dev/null 2>&1 || exit 1; "
    "for p in /dev/i2c-*; do [ -c \"$p\" ] && exit 0; done; exit 1";

constexpr std::string_view kSpiProbe =
    "for p in /dev/spidev*; do [ -c \"$p\" ] && exit 0; done; exit 1";

constexpr std::string_view kUartProbe =
    "for p in /dev/ttyAMA* /dev/ttyS* /dev/serial*; do "
    "([ -c \"$p\" ] || [ -L \"$p\" ]) && exit 0; "
    "done; exit 1";

std::string_view probeCommand(LinuxHardwareCapability capability) {
    switch (capability) {
    case LinuxHardwareCapability::Gpio:
        return kGpioProbe;

    case LinuxHardwareCapability::I2c:
        return kI2cProbe;

    case LinuxHardwareCapability::Spi:
        return kSpiProbe;

    case LinuxHardwareCapability::Uart:
        return kUartProbe;
    }
    return {};
}

} // namespace

bool hasLinuxHardwareCapability(
    SystemReader& reader,
    LinuxHardwareCapability capability
) {
    const auto result = reader.execute(
        std::string(kToolPath) + std::string(probeCommand(capability)));
    return result.succeeded();
}

} // namespace system_monitor