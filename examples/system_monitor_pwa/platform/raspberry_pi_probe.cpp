#include "raspberry_pi_probe.hpp"

#include <charconv>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace system_monitor {

namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

} // namespace

RaspberryPiProbe::RaspberryPiProbe(std::shared_ptr<SystemReader> reader)
    : reader_(std::move(reader))
{
    if (!reader_) {
        throw std::invalid_argument("RaspberryPiProbe requires a SystemReader");
    }
}

std::optional<RaspberryPiInfo> RaspberryPiProbe::read() const {
    std::istringstream cpuInfo(reader_->readFile("/proc/cpuinfo"));
    std::string line;
    RaspberryPiInfo info;

    while (std::getline(cpuInfo, line)) {
        const auto colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }

        const auto key = trim(line.substr(0, colon));
        const auto value = trim(line.substr(colon + 1));
        if (key == "Model") {
            info.model = value;
        } else if (key == "Revision") {
            info.revision = value;
        } else if (key == "Serial") {
            info.serial = value;
        }
    }

    if (info.model.rfind("Raspberry Pi", 0) != 0) {
        return std::nullopt;
    }

    const auto temperature = reader_->execute(
        "cat /sys/class/thermal/thermal_zone0/temp");
    if (temperature.succeeded()) {
        const auto text = trim(temperature.standardOutput);
        std::uint64_t milliCelsius = 0;
        const auto parsed = std::from_chars(
            text.data(),
            text.data() + text.size(),
            milliCelsius);
        if (parsed.ec == std::errc{} &&
            parsed.ptr == text.data() + text.size()) {
            info.temperatureCelsius =
                static_cast<double>(milliCelsius) / 1000.0;
        }
    }

    return info;
}

} // namespace system_monitor