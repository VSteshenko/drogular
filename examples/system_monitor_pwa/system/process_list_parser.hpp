#pragma once

#include "system_snapshot.hpp"

#include <string_view>
#include <vector>

namespace system_monitor {

[[nodiscard]] std::vector<ProcessInfo> parseProcessList(std::string_view input);

} // namespace system_monitor