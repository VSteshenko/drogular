#include "process_service.hpp"

#include <stdexcept>
#include <utility>

namespace system_monitor {

ProcessService::ProcessService(
    std::shared_ptr<SystemMetricsProvider> provider,
    std::chrono::milliseconds refreshInterval)
    : provider_(std::move(provider)),
      refreshInterval_(refreshInterval)
{
    if (!provider_) {
        throw std::invalid_argument("ProcessService requires a metrics provider");
    }
    if (refreshInterval_ < std::chrono::milliseconds::zero()) {
        throw std::invalid_argument("ProcessService refresh interval cannot be negative");
    }
    statistics_.refreshInterval = refreshInterval_;
}

std::vector<ProcessInfo> ProcessService::snapshot() {
    std::lock_guard lock(mutex_);

    const auto now = SteadyClock::now();
    if (hasSnapshot_ && now - lastRefreshAttempt_ < refreshInterval_) {
        ++statistics_.cacheHits;
        return cachedProcesses_;
    }

    const auto startedAt = SteadyClock::now();
    try {
        auto processes = provider_->processes();
        const auto completedAt = SteadyClock::now();

        cachedProcesses_ = std::move(processes);
        hasSnapshot_ = true;
        lastRefreshAttempt_ = completedAt;
        lastSuccessfulRefresh_ = completedAt;
        ++statistics_.updates;
        statistics_.lastUpdateDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(completedAt - startedAt);
        statistics_.lastSuccessfulUpdate = std::chrono::system_clock::now();
        statistics_.snapshotAge = std::chrono::milliseconds::zero();
        statistics_.healthy = true;
        return cachedProcesses_;
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
            return cachedProcesses_;
        }
        throw;
    }
}

ProcessStatistics ProcessService::statistics() const {
    std::lock_guard lock(mutex_);
    auto result = statistics_;
    if (hasSnapshot_) {
        result.snapshotAge = std::chrono::duration_cast<std::chrono::milliseconds>(
            SteadyClock::now() - lastSuccessfulRefresh_);
    }
    return result;
}

} // namespace system_monitor