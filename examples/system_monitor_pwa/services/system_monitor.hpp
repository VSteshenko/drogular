#pragma once

#include "system/system_metrics_provider.hpp"

#include <memory>
#include <vector>

namespace system_monitor {

class SystemMonitor {
public:
    explicit SystemMonitor(std::shared_ptr<SystemMetricsProvider> provider);

    [[nodiscard]] SystemSnapshot snapshot();
    [[nodiscard]] std::vector<ProcessInfo> processes();

private:
    std::shared_ptr<SystemMetricsProvider> provider_;
};

} // namespace system_monitor