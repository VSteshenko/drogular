#include "spi_gpio_correlator.hpp"

#include <string>

namespace system_monitor {

std::vector<SpiGpioPin> SpiGpioCorrelator::pinsForBus(
    std::uint32_t busNumber,
    const GpioSnapshot& gpioSnapshot
) {
    const auto functionPrefix = "SPI" + std::to_string(busNumber) + "_";
    const auto consumerPrefix = "spi" + std::to_string(busNumber) + " CS";
    std::vector<SpiGpioPin> result;
    for (const auto& chip : gpioSnapshot.chips) {
        for (const auto& line : chip.lines) {
            std::string role;
            if (line.function.starts_with(functionPrefix)) {
                auto suffix = line.function.substr(functionPrefix.size());
                if (suffix == "MOSI") role = "mosi";
                else if (suffix == "MISO") role = "miso";
                else if (suffix == "SCLK") role = "sclk";
                else if (suffix.starts_with("CE")) {
                    auto chipSelect = suffix.substr(2);
                    if (chipSelect.ends_with("_N")) chipSelect.resize(chipSelect.size() - 2);
                    role = "ce" + chipSelect;
                }
            }
            if (role.empty() && line.consumer.starts_with(consumerPrefix)) {
                const auto chipSelect = line.consumer.substr(consumerPrefix.size());
                if (!chipSelect.empty() && chipSelect.find_first_not_of("0123456789") == std::string::npos) {
                    role = "ce" + chipSelect;
                }
            }
            if (role.empty()) continue;
            result.push_back(SpiGpioPin{
                role,
                chip.chip.name,
                line.offset,
                line.name,
                line.function,
                line.consumer
            });
        }
    }

    return result;
}

} // namespace system_monitor