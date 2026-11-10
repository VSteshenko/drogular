#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>

namespace drogular {

/** Metrics describe the server process, never an individual Job. */
struct ProcessMetricsSnapshot {
    bool available = false;
    // Percentage of one logical CPU: 100% means one fully busy core.
    std::optional<double> cpuPercent;
    std::uint64_t residentBytes = 0;
    std::uint64_t peakResidentBytes = 0;
    std::uint64_t threadCount = 0;
    std::uint64_t uptimeMs = 0;
    std::uint64_t cpuTimeMs = 0;
};

/**
 * Platform-backed, on-demand process metrics sampler.
 *
 * The first sample has no CPU percentage; subsequent samples report CPU
 * usage over the interval since the preceding call. snapshot() serializes
 * sampling to keep the previous sample consistent between HTTP requests.
 */
class ProcessMetricsProvider {
public:
    ProcessMetricsProvider() = default;
    ProcessMetricsProvider(const ProcessMetricsProvider&) = delete;
    ProcessMetricsProvider& operator=(const ProcessMetricsProvider&) = delete;

    ProcessMetricsSnapshot snapshot() const noexcept;

private:
    mutable std::mutex mutex_;
    mutable std::optional<std::chrono::steady_clock::time_point> lastSample_;
    mutable std::uint64_t lastCpuUs_ = 0;
};

} // namespace drogular