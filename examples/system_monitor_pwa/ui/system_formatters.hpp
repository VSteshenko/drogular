#pragma once

#include <cstdint>
#include <string>

namespace system_monitor::ui {

[[nodiscard]] std::string formatBytes(std::uint64_t bytes);
[[nodiscard]] std::string formatDuration(std::uint64_t seconds);
[[nodiscard]] std::string formatPercent(double value);
[[nodiscard]] double percentOf(std::uint64_t used, std::uint64_t total);

} // namespace system_monitor::ui