#pragma once

#include "spi/spi_model.hpp"
#include "spi/spi_provider.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>

namespace system_monitor {

struct SpiServiceStatistics {
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

class SpiService {
public:
    explicit SpiService(std::shared_ptr<SpiProvider> provider,
                        std::chrono::milliseconds refreshInterval = std::chrono::seconds(30));
    [[nodiscard]] SpiSnapshot snapshot();
    [[nodiscard]] SpiServiceStatistics statistics() const;
    [[nodiscard]] bool available() const noexcept;
private:
    using SteadyClock = std::chrono::steady_clock;
    std::shared_ptr<SpiProvider> provider_;
    std::chrono::milliseconds refreshInterval_;
    mutable std::mutex mutex_;
    SpiSnapshot cachedSnapshot_;
    SpiServiceStatistics statistics_;
    SteadyClock::time_point lastRefreshAttempt_{};
    SteadyClock::time_point lastSuccessfulRefresh_{};
    bool hasSnapshot_{false};
};

} // namespace system_monitor