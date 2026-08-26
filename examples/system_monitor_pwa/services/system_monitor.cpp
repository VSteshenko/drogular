#include "system_monitor.hpp"

#include <stdexcept>
#include <utility>

namespace system_monitor {

SystemMonitor::SystemMonitor(
    std::shared_ptr<SystemMetricsProvider> provider,
    std::chrono::milliseconds refreshInterval
)
    : provider_(std::move(provider)),
      refreshInterval_(refreshInterval)
{
    if (provider_ == nullptr) {
        throw std::invalid_argument("SystemMonitor requires a metrics provider");
    }
    if (refreshInterval_ < std::chrono::milliseconds::zero()) {
        throw std::invalid_argument("SystemMonitor refresh interval cannot be negative");
    }

    statistics_.refreshInterval = refreshInterval_;
}

SystemSnapshot SystemMonitor::snapshot() {
    std::lock_guard lock(mutex_);

    const auto now = SteadyClock::now();
    if (hasSnapshot_ && now - lastRefreshAttempt_ < refreshInterval_) {
        ++statistics_.cacheHits;
        return cachedSnapshot_;
    }

    const auto startedAt = SteadyClock::now();

    try {
        auto snapshot = provider_->snapshot();
        const auto completedAt = SteadyClock::now();

        lastRefreshAttempt_ = completedAt;
        cachedSnapshot_ = std::move(snapshot);
        hasSnapshot_ = true;
        lastSuccessfulRefresh_ = completedAt;

        ++statistics_.updates;
        statistics_.lastUpdateDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(completedAt - startedAt);
        statistics_.lastSuccessfulUpdate = std::chrono::system_clock::now();
        statistics_.snapshotAge = std::chrono::milliseconds::zero();
        statistics_.healthy = true;

        return cachedSnapshot_;
    } catch (...) {
        const auto completedAt = SteadyClock::now();
        lastRefreshAttempt_ = completedAt;
        ++statistics_.failedUpdates;
        statistics_.lastUpdateDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(completedAt - startedAt);
        statistics_.healthy = false;

        if (hasSnapshot_) {
            statistics_.snapshotAge =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    completedAt - lastSuccessfulRefresh_);
            return cachedSnapshot_;
        }

        throw;
    }
}

std::vector<ProcessInfo> SystemMonitor::processes() {
    return provider_->processes();
}

MonitorStatistics SystemMonitor::statistics() const {
    std::lock_guard lock(mutex_);

    auto result = statistics_;
    if (hasSnapshot_) {
        result.snapshotAge = std::chrono::duration_cast<std::chrono::milliseconds>(
            SteadyClock::now() - lastSuccessfulRefresh_);
    }

    return result;
}

} // namespace system_monitor