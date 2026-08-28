#pragma once

#include "system/system_metrics_provider.hpp"
#include "system/system_reader.hpp"

#include <memory>
#include <mutex>
#include <optional>

namespace system_monitor {

class LinuxSystemMetricsProvider final : public SystemMetricsProvider {
public:
    explicit LinuxSystemMetricsProvider(std::shared_ptr<SystemReader> reader);

    [[nodiscard]] SystemSnapshot snapshot() override;
    [[nodiscard]] std::vector<ProcessInfo> processes() override;

private:
    struct CpuTicks {
        std::uint64_t active{0};
        std::uint64_t idle{0};
    };

    [[nodiscard]] CpuInfo readCpu();
    [[nodiscard]] std::optional<RaspberryPiInfo> readRaspberryPi() const;
    [[nodiscard]] MemoryInfo readMemory() const;
    [[nodiscard]] std::vector<DiskInfo> readDisks() const;
    [[nodiscard]] SystemInfo readSystem() const;
    [[nodiscard]] CpuTicks readCpuTicks() const;

    std::shared_ptr<SystemReader> reader_;
    std::mutex cpuMutex_;
    std::optional<CpuTicks> previousCpuTicks_;
};

} // namespace system_monitor