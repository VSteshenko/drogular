#include "i2c_gpio_correlator.hpp"

#include <string>

namespace system_monitor {

std::vector<I2cGpioPin> I2cGpioCorrelator::pinsForBus(
    std::uint32_t busNumber,
    const GpioSnapshot& gpioSnapshot,
    const BoardGpioMetadata& boardMetadata
) {
    const auto suffix = std::to_string(busNumber);
    const auto sdaFunction = "SDA" + suffix;
    const auto sclFunction = "SCL" + suffix;

    std::vector<I2cGpioPin> result;

    for (const auto& chip : gpioSnapshot.chips) {
        for (const auto& line : chip.lines) {
            I2cGpioRole role;
            if (line.function == sdaFunction) {
                role = I2cGpioRole::Sda;
            } else if (line.function == sclFunction) {
                role = I2cGpioRole::Scl;
            } else {
                continue;
            }

            const auto metadata = boardMetadata.line(chip.chip.name, line.offset);
            result.push_back(I2cGpioPin{
                .role = role,
                .chip = chip.chip.name,
                .offset = line.offset,
                .name = line.name,
                .function = line.function,
                .exposure = metadata.exposure,
                .physicalHeaderPin = metadata.physicalHeaderPin
            });
        }
    }

    return result;
}

} // namespace system_monitor