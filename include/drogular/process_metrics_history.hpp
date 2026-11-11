#pragma once

#include <drogular/process_metrics.hpp>

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

namespace drogular {

/** One recorded sample of process-wide metrics, relative to sampler start. */
struct ProcessMetricsHistorySample {
    std::uint64_t elapsedMs = 0;
    ProcessMetricsSnapshot metrics;
};

/**
 * Bounded, process-wide history sampler. No Job Runtime or HTTP dependency.
 * start()/stop() are idempotent. The owning App starts it when serving.
 */
class ProcessMetricsHistory {
public:
    explicit ProcessMetricsHistory(
        ProcessMetricsProvider& provider,
        std::chrono::milliseconds interval = std::chrono::seconds(2),
        std::size_t capacity = 150);
    ~ProcessMetricsHistory();

    ProcessMetricsHistory(const ProcessMetricsHistory&) = delete;
    ProcessMetricsHistory& operator=(const ProcessMetricsHistory&) = delete;

    void start();
    void stop() noexcept;
    std::vector<ProcessMetricsHistorySample> snapshot() const;

private:
    void collect();
    void run();

    ProcessMetricsProvider& provider_;
    const std::chrono::milliseconds interval_;
    const std::size_t capacity_;
    mutable std::mutex mutex_;
    std::condition_variable wake_;
    std::deque<ProcessMetricsHistorySample> samples_;
    std::chrono::steady_clock::time_point started_{};
    std::thread worker_;
    bool running_ = false;
    bool stopping_ = false;
};

} // namespace drogular