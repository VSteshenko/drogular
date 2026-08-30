#include "uart_service.hpp"

#include <stdexcept>
#include <utility>

namespace system_monitor {

UartService::UartService(std::shared_ptr<UartProvider> provider,
                         std::chrono::milliseconds refreshInterval)
    : provider_(std::move(provider)),
      refreshInterval_(refreshInterval)
{
    if (refreshInterval_ < std::chrono::milliseconds::zero()) {
        throw std::invalid_argument("UartService refresh interval cannot be negative");
    }
    statistics_.refreshInterval = refreshInterval_;
    statistics_.available = provider_ != nullptr;
}

UartSnapshot UartService::snapshot() {
    std::lock_guard lock(mutex_);
    if (!provider_) {
        return {};
    }

    const auto now = SteadyClock::now();
    if (hasSnapshot_ && now - lastRefreshAttempt_ < refreshInterval_) {
        ++statistics_.cacheHits;
        return cachedSnapshot_;
    }

    const auto startedAt = SteadyClock::now();
    try {
        UartSnapshot snapshot{.devices = provider_->devices()};
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
                    completedAt - lastSuccessfulRefresh_
                );
            return cachedSnapshot_;
        }

        throw;
    }
}

UartServiceStatistics UartService::statistics() const {
    std::lock_guard lock(mutex_);
    auto result = statistics_;
    if (hasSnapshot_) {
        result.snapshotAge =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                SteadyClock::now() - lastSuccessfulRefresh_
            );
    }

    return result;
}

bool UartService::available() const noexcept {
    return provider_ != nullptr;
}

} // namespace system_monitor