#include "spidev_spi_provider.hpp"

#include <charconv>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace system_monitor {

namespace {

constexpr std::string_view kListCommand =
    "for p in /dev/spidev*; do [ -e \"$p\" ] && basename \"$p\"; done";

bool parseUnsigned(std::string_view text, std::uint32_t& value) {
    if (text.empty()) return false;
    const auto [end, error] =
        std::from_chars(text.data(), text.data() + text.size(), value);

    return error == std::errc{} && end == text.data() + text.size();
}

} // namespace

SpidevSpiProvider::SpidevSpiProvider(std::shared_ptr<SystemReader> reader)
    : reader_(std::move(reader)) {
    if (!reader_) throw std::invalid_argument("SpidevSpiProvider requires a SystemReader");
}

std::vector<SpiDeviceInfo> SpidevSpiProvider::devices() {
    const auto result = reader_->execute(kListCommand);
    if (!result.succeeded()) {
        throw std::runtime_error("SPI device inventory failed with exit code " +
                                 std::to_string(result.exitCode) + ": " +
                                 (!result.standardError.empty() ? result.standardError : result.standardOutput));
    }

    std::vector<SpiDeviceInfo> devices;
    std::string_view output(result.standardOutput);
    while (!output.empty()) {
        const auto newline = output.find('\n');
        auto line = output.substr(0, newline);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);

        constexpr std::string_view prefix = "spidev";
        if (line.starts_with(prefix)) {
            const auto separator = line.find('.', prefix.size());
            if (separator != std::string_view::npos) {
                std::uint32_t bus = 0;
                std::uint32_t chipSelect = 0;
                if (parseUnsigned(line.substr(prefix.size(), separator - prefix.size()), bus) &&
                    parseUnsigned(line.substr(separator + 1), chipSelect)) {
                    devices.push_back(SpiDeviceInfo{
                        .bus = bus,
                        .chipSelect = chipSelect,
                        .path = "/dev/" + std::string(line)
                    });
                }
            }
        }

        if (newline == std::string_view::npos) break;
        output.remove_prefix(newline + 1);
    }
    return devices;
}

} // namespace system_monitor