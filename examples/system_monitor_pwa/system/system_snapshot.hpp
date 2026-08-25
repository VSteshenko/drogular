#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace system_monitor {

struct CpuInfo {
    double usagePercent{0.0};
    std::uint32_t logicalCores{0};
    double load1{0.0};
    double load5{0.0};
    double load15{0.0};
    std::optional<double> temperatureCelsius;
};

struct MemoryInfo {
    std::uint64_t totalBytes{0};
    std::uint64_t usedBytes{0};
    std::uint64_t availableBytes{0};
};

struct DiskInfo {
    std::string device;
    std::string mountPoint;
    std::string fileSystem;
    std::uint64_t totalBytes{0};
    std::uint64_t usedBytes{0};
    std::uint64_t availableBytes{0};
};

struct SystemInfo {
    std::string hostname;
    std::string operatingSystem;
    std::string kernel;
    std::string architecture;
    std::uint64_t uptimeSeconds{0};
};

struct RaspberryPiInfo {
    std::string model;
    std::string revision;
    std::string serial;
    std::optional<double> temperatureCelsius;
};

struct ProcessInfo {
    std::int64_t pid{0};
    std::string user;
    std::string name;
    std::string command;
    double cpuPercent{0.0};
    double memoryPercent{0.0};
    std::uint64_t residentBytes{0};
};

struct SystemSnapshot {
    CpuInfo cpu;
    MemoryInfo memory;
    std::vector<DiskInfo> disks;
    SystemInfo system;
    std::optional<RaspberryPiInfo> raspberryPi;
};

} // namespace system_monitor