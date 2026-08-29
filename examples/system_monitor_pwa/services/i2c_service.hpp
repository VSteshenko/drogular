#pragma once

#include "i2c/i2c_model.hpp"
#include "i2c/i2c_provider.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>

namespace system_monitor {

struct I2cServiceStatistics {
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

class I2cService {
public:
    explicit I2cService(
        std::shared_ptr<I2cProvider> provider,
        std::chrono::milliseconds refreshInterval = std::chrono::minutes(5));

    [[nodiscard]] I2cSnapshot snapshot();
    [[nodiscard]] I2cServiceStatistics statistics() const;
    [[nodiscard]] bool available() const noexcept;

private:
    using SteadyClock = std::chrono::steady_clock;

    std::shared_ptr<I2cProvider> provider_;
    std::chrono::milliseconds refreshInterval_;

    mutable std::mutex mutex_;
    I2cSnapshot cachedSnapshot_;
    I2cServiceStatistics statistics_;
    SteadyClock::time_point lastRefreshAttempt_{};
    SteadyClock::time_point lastSuccessfulRefresh_{};
    bool hasSnapshot_{false};
};

} // namespace system_monitor