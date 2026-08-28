#include "system_formatters.hpp"

#include <algorithm>
#include <array>
#include <iomanip>
#include <sstream>

namespace system_monitor::ui {

std::string formatBytes(std::uint64_t bytes) {
    constexpr double unit = 1024.0;
    constexpr std::array<const char*, 5> suffixes{
        "B", "KiB", "MiB", "GiB", "TiB"
    };

    double value = static_cast<double>(bytes);
    std::size_t suffix = 0;
    while (value >= unit && suffix + 1 < suffixes.size()) {
        value /= unit;
        ++suffix;
    }

    std::ostringstream stream;
    if (suffix == 0) {
        stream << bytes;
    } else {
        stream << std::fixed << std::setprecision(value >= 10.0 ? 1 : 2) << value;
    }
    stream << ' ' << suffixes[suffix];
    return stream.str();
}

std::string formatDuration(std::uint64_t seconds) {
    const auto days = seconds / 86400;
    seconds %= 86400;
    const auto hours = seconds / 3600;
    seconds %= 3600;
    const auto minutes = seconds / 60;

    std::ostringstream stream;
    if (days != 0) {
        stream << days << "d ";
    }
    if (days != 0 || hours != 0) {
        stream << hours << "h ";
    }
    stream << minutes << "m";
    return stream.str();
}

std::string formatPercent(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1)
           << std::clamp(value, 0.0, 100.0) << '%';
    return stream.str();
}

std::string formatTemperature(double celsius) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1)
           << celsius << " °C";
    return stream.str();
}

double percentOf(std::uint64_t used, std::uint64_t total) {
    if (total == 0) {
        return 0.0;
    }
    return std::clamp(
        100.0 * static_cast<double>(used) / static_cast<double>(total),
        0.0,
        100.0);
}

} // namespace system_monitor::ui