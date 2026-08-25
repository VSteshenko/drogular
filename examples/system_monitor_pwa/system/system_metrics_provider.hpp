#pragma once

#include "system_snapshot.hpp"

#include <vector>

namespace system_monitor {

class SystemMetricsProvider {
public:
    virtual ~SystemMetricsProvider() = default;

    [[nodiscard]] virtual SystemSnapshot snapshot() = 0;
    [[nodiscard]] virtual std::vector<ProcessInfo> processes() = 0;
};

} // namespace system_monitor