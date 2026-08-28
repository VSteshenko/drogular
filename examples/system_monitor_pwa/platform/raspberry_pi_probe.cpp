#include "raspberry_pi_probe.hpp"

#include <charconv>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
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


std::optional<std::uint64_t> parseUnsigned(
    std::string_view value,
    int base = 10
) {
    std::uint64_t result = 0;
    const auto parsed = std::from_chars(
        value.data(),
        value.data() + value.size(),
        result,
        base);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != value.data() + value.size()) {
        return std::nullopt;
    }
    return result;
}

std::optional<RaspberryPiHealth> parseThrottled(std::string value) {
    value = trim(std::move(value));
    constexpr std::string_view prefix{"throttled=0x"};
    if (!value.starts_with(prefix)) {
        return std::nullopt;
    }

    const auto bits = parseUnsigned(
        std::string_view(value).substr(prefix.size()),
        16);
    if (!bits) {
        return std::nullopt;
    }

    RaspberryPiHealth health;
    health.underVoltage = (*bits & (1ULL << 0)) != 0;
    health.frequencyCapped = (*bits & (1ULL << 1)) != 0;
    health.throttled = (*bits & (1ULL << 2)) != 0;
    health.softTemperatureLimit = (*bits & (1ULL << 3)) != 0;

    health.underVoltageOccurred = (*bits & (1ULL << 16)) != 0;
    health.frequencyCappingOccurred = (*bits & (1ULL << 17)) != 0;
    health.throttlingOccurred = (*bits & (1ULL << 18)) != 0;
    health.softTemperatureLimitOccurred = (*bits & (1ULL << 19)) != 0;

    return health;
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
        if (const auto milliCelsius = parseUnsigned(text)) {
            info.temperatureCelsius =
                static_cast<double>(*milliCelsius) / 1000.0;
        }
    }

    const auto frequency = reader_->execute(
        "cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (frequency.succeeded()) {
        const auto text = trim(frequency.standardOutput);
        if (const auto kiloHertz = parseUnsigned(text)) {
            constexpr std::uint64_t scale = 1000;
            if (*kiloHertz <=
                std::numeric_limits<std::uint64_t>::max() / scale) {
                info.cpuFrequencyHz = *kiloHertz * scale;
            }
        }
    }

    const auto throttled = reader_->execute("vcgencmd get_throttled");
    if (throttled.succeeded()) {
        info.health = parseThrottled(throttled.standardOutput);
    }

    return info;
}

} // namespace system_monitor