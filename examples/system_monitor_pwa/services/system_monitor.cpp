#include "system_monitor.hpp"

#include <stdexcept>
#include <utility>

namespace system_monitor {

SystemMonitor::SystemMonitor(std::shared_ptr<SystemMetricsProvider> provider)
    : provider_(std::move(provider)) {
    if (provider_ == nullptr) {
        throw std::invalid_argument("SystemMonitor requires a metrics provider");
    }
}

SystemSnapshot SystemMonitor::snapshot() {
    return provider_->snapshot();
}

std::vector<ProcessInfo> SystemMonitor::processes() {
    return provider_->processes();
}

} // namespace system_monitor