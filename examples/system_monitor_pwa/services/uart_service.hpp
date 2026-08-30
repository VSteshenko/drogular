#pragma once

#include "uart/uart_model.hpp"
#include "uart/uart_provider.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>

namespace system_monitor {

struct UartServiceStatistics {
    std::uint64_t updates{0};
    std::uint64_t cacheHits{0};
    std::uint64_t failedUpdates{0};
    std::chrono::milliseconds lastUpdateDuration{0};
    std::chrono::system_clock::time_point lastSuccessfulUpdate{};
    std::chrono::milliseconds snapshotAge{0};
    std::chrono::milliseconds refreshInterval{0};
    bool available{false};
    bool healthy{false};
};

class UartService {
public:
    explicit UartService(std::shared_ptr<UartProvider> provider,
                         std::chrono::milliseconds refreshInterval = std::chrono::seconds(30));
    [[nodiscard]] UartSnapshot snapshot();
    [[nodiscard]] UartServiceStatistics statistics() const;
    [[nodiscard]] bool available() const noexcept;

private:
    using SteadyClock = std::chrono::steady_clock;
    std::shared_ptr<UartProvider> provider_;
    std::chrono::milliseconds refreshInterval_;
    mutable std::mutex mutex_;
    UartSnapshot cachedSnapshot_;
    UartServiceStatistics statistics_;
    SteadyClock::time_point lastRefreshAttempt_{};
    SteadyClock::time_point lastSuccessfulRefresh_{};
    bool hasSnapshot_{false};
};

} // namespace system_monitor