#pragma once

#include "system/system_metrics_provider.hpp"

#include <cstdint>
#include <mutex>
#include <optional>

namespace system_monitor {

class MacOsSystemMetricsProvider final : public SystemMetricsProvider {
public:
    [[nodiscard]] SystemSnapshot snapshot() override;
    [[nodiscard]] std::vector<ProcessInfo> processes() override;

private:
    struct CpuTicks {
        std::uint64_t active{0};
        std::uint64_t idle{0};
    };

    [[nodiscard]] CpuInfo readCpu();
    [[nodiscard]] MemoryInfo readMemory() const;
    [[nodiscard]] std::vector<DiskInfo> readDisks() const;
    [[nodiscard]] SystemInfo readSystem() const;
    [[nodiscard]] CpuTicks readCpuTicks() const;

    std::mutex cpuMutex_;
    std::optional<CpuTicks> previousCpuTicks_;
};

} // namespace system_monitor