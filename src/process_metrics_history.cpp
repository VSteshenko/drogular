#include <drogular/process_metrics_history.hpp>

#include <stdexcept>
#include <utility>

namespace drogular {

ProcessMetricsHistory::ProcessMetricsHistory(ProcessMetricsProvider& provider,
                                           std::chrono::milliseconds interval,
                                           std::size_t capacity)
    : provider_(provider), interval_(interval), capacity_(capacity)
{
    if (interval_ <= std::chrono::milliseconds::zero() || capacity_ == 0) {
        throw std::invalid_argument("ProcessMetricsHistory requires a positive interval and capacity");
    }
}

ProcessMetricsHistory::~ProcessMetricsHistory() {
    stop();
}

void ProcessMetricsHistory::start() {
    std::lock_guard lock(mutex_);
    if (running_) {
        return;
    }
    samples_.clear();
    started_ = std::chrono::steady_clock::now();
    stopping_ = false;
    running_ = true;
    try {
        worker_ = std::thread([this] { run(); });
    } catch (...) {
        running_ = false;
        throw;
    }
}

void ProcessMetricsHistory::stop() noexcept {
    std::thread worker;
    {
        std::lock_guard lock(mutex_);
        if (!running_) {
            return;
        }
        stopping_ = true;
        wake_.notify_all();
        worker = std::move(worker_);
    }
    if (worker.joinable()) {
        worker.join();
    }
    std::lock_guard lock(mutex_);
    running_ = false;
}

std::vector<ProcessMetricsHistorySample> ProcessMetricsHistory::snapshot() const {
    std::lock_guard lock(mutex_);
    return {
        samples_.begin(),
        samples_.end()
    };
}

void ProcessMetricsHistory::collect() {
    auto metrics = provider_.snapshot();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - started_).count();
    std::lock_guard lock(mutex_);
    if (stopping_) {
        return;
    }
    if (samples_.size() == capacity_) {
        samples_.pop_front();
    }
    samples_.push_back({
        static_cast<std::uint64_t>(elapsed),
        std::move(metrics)
    });
}

void ProcessMetricsHistory::run() {
    auto deadline = std::chrono::steady_clock::now();
    for (;;) {
        try {
            collect();
        } catch (...) {
            // Resource monitoring must never bring down the application.
        }
        deadline += interval_;
        std::unique_lock lock(mutex_);
        if (wake_.wait_until(lock, deadline, [this] { return stopping_; })) {
            break;
        }
    }
}

} // namespace drogular