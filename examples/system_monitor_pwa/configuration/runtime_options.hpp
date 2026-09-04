#pragma once

#include "system/monitor_target.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace system_monitor {

struct RuntimeOptions {
    MonitorTarget target{MonitorTarget::local()};
    std::vector<std::uint32_t> i2cScanBuses;
    bool showHelp{false};
};

using EnvironmentLookup =
    std::function<std::optional<std::string>(std::string_view name)>;

[[nodiscard]] RuntimeOptions parseRuntimeOptions(
    int argc,
    const char* const argv[],
    EnvironmentLookup environment = {});

[[nodiscard]] std::string runtimeOptionsHelp(std::string_view executableName);

} // namespace system_monitor