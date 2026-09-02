#pragma once

#include "system/system_metrics_provider.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace system_monitor {

struct ProcessStatistics {
    std::uint64_t updates{0};
    std::uint64_t cacheHits{0};
    std::uint64_t failedUpdates{0};
    std::chrono::milliseconds lastUpdateDuration{0};
    std::chrono::system_clock::time_point lastSuccessfulUpdate{};
    std::chrono::milliseconds snapshotAge{0};
    std::chrono::milliseconds refreshInterval{0};
    bool healthy{false};
};

class ProcessService {
public:
    explicit ProcessService(
        std::shared_ptr<SystemMetricsProvider> provider,
        std::chrono::milliseconds refreshInterval = std::chrono::seconds(2));

    [[nodiscard]] std::vector<ProcessInfo> snapshot();
    [[nodiscard]] ProcessStatistics statistics() const;

private:
    using SteadyClock = std::chrono::steady_clock;

    std::shared_ptr<SystemMetricsProvider> provider_;
    std::chrono::milliseconds refreshInterval_;

    mutable std::mutex mutex_;
    std::vector<ProcessInfo> cachedProcesses_;
    bool hasSnapshot_{false};
    SteadyClock::time_point lastRefreshAttempt_{};
    SteadyClock::time_point lastSuccessfulRefresh_{};
    ProcessStatistics statistics_;
};

} // namespace system_monitor