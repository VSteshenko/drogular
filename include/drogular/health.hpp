#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace drogular {

enum class HealthStatus { Healthy, Degraded, Unhealthy };

struct HealthCheckResult {
    HealthStatus status = HealthStatus::Healthy;
    std::string message;
};

struct HealthCheckReport {
    std::string name;
    HealthCheckResult result;
    std::chrono::milliseconds duration{0};
};

struct HealthReport {
    HealthStatus status = HealthStatus::Healthy;
    std::vector<HealthCheckReport> checks;
};

/** Independent application readiness registry. Checks must be thread-safe. */
class HealthRegistry {
public:
    using Check = std::function<HealthCheckResult()>;

    /** Register before starting the HTTP server. Duplicate/empty names are rejected. */
    void add(std::string name, Check check,
             std::chrono::milliseconds timeout = std::chrono::seconds(2));

    /** A readiness report. A timed-out check cannot be launched again while running. */
    [[nodiscard]] HealthReport readiness() const;

private:
    struct Entry;
    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<Entry>> entries_;
};

[[nodiscard]] const char* healthStatusName(HealthStatus status) noexcept;

} // namespace drogular