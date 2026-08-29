#pragma once

#include "gpio/gpio_provider.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace system_monitor {

struct GpioChipSnapshot {
    GpioChipInfo chip;
    std::vector<GpioLineInfo> lines;
};

struct GpioSnapshot {
    std::vector<GpioChipSnapshot> chips;
};

struct GpioServiceStatistics {
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

class GpioService {
public:
    explicit GpioService(
        std::shared_ptr<GpioProvider> provider = nullptr,
        std::chrono::milliseconds refreshInterval = std::chrono::seconds(30));

    [[nodiscard]] GpioSnapshot snapshot();
    [[nodiscard]] GpioServiceStatistics statistics() const;
    [[nodiscard]] bool available() const noexcept;

private:
    using SteadyClock = std::chrono::steady_clock;

    std::shared_ptr<GpioProvider> provider_;
    std::chrono::milliseconds refreshInterval_;

    mutable std::mutex mutex_;
    GpioSnapshot cachedSnapshot_;
    bool hasSnapshot_{false};
    SteadyClock::time_point lastRefreshAttempt_{};
    SteadyClock::time_point lastSuccessfulRefresh_{};
    GpioServiceStatistics statistics_;
};

} // namespace system_monitor