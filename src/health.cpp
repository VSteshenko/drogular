#include <drogular/health.hpp>

#include <atomic>
#include <future>
#include <stdexcept>
#include <thread>
#include <utility>

namespace drogular {

struct HealthRegistry::Entry {
    std::string name;
    Check check;
    std::chrono::milliseconds timeout;
    std::atomic<bool> running{false};
};

const char* healthStatusName(HealthStatus status) noexcept {
    switch (status) {
    case HealthStatus::Healthy:
        return "healthy";
    case HealthStatus::Degraded:
        return "degraded";
    case HealthStatus::Unhealthy:
        return "unhealthy";
    }

    return "unhealthy";
}

void HealthRegistry::add(std::string name, Check check,
                         std::chrono::milliseconds timeout
) {
    if (name.empty() || !check || timeout.count() <= 0) {
        throw std::invalid_argument("Health check requires a name, callback and positive timeout");
    }
    std::lock_guard lock(mutex_);
    for (const auto& entry : entries_) {
        if (entry->name == name) {
            throw std::invalid_argument("Duplicate health check name: " + name);
        }
    }

    auto entry = std::make_shared<Entry>();
    entry->name = std::move(name);
    entry->check = std::move(check);
    entry->timeout = timeout;
    entries_.push_back(std::move(entry));
}

HealthReport HealthRegistry::readiness() const {
    std::vector<std::shared_ptr<Entry>> entries;
    {
        std::lock_guard lock(mutex_);
        entries = entries_;
    }

    HealthReport report;
    for (const auto& entry : entries) {
        const auto start = std::chrono::steady_clock::now();
        HealthCheckResult result;
        bool expected = false;
        if (!entry->running.compare_exchange_strong(expected, true)) {
            result = {
                HealthStatus::Unhealthy,
                "check already running"
            };
        } else {
            auto promise = std::make_shared<std::promise<HealthCheckResult>>();
            auto future = promise->get_future();
            try {
                std::thread([entry, promise] {
                    HealthCheckResult value;
                    try {
                        value = entry->check();
                    } catch (const std::exception&) {
                        value = {HealthStatus::Unhealthy, "check threw an exception"};
                    } catch (...) {
                        value = {HealthStatus::Unhealthy, "check failed"};
                    }
                    // Publish before releasing the in-flight slot.
                    promise->set_value(std::move(value));
                    entry->running.store(false);
                }).detach();
                if (future.wait_for(entry->timeout) == std::future_status::ready) {
                    result = future.get();
                } else {
                    result = {HealthStatus::Unhealthy, "check timed out"};
                }
            } catch (...) {
                entry->running.store(false);
                result = {HealthStatus::Unhealthy, "unable to start check"};
            }
        }
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start
        );
        report.checks.push_back({entry->name, std::move(result), duration});
        const auto status = report.checks.back().result.status;
        if (status == HealthStatus::Unhealthy) {
            report.status = HealthStatus::Unhealthy;
        } else if (status == HealthStatus::Degraded && report.status == HealthStatus::Healthy) {
            report.status = HealthStatus::Degraded;
        }
    }
    return report;
}

} // namespace drogular