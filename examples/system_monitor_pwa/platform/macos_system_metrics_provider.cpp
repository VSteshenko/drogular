#include "macos_system_metrics_provider.hpp"
#include "system/process_list_parser.hpp"

#if !defined(__APPLE__)
#error "MacOsSystemMetricsProvider can only be built on macOS"
#endif

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

#include <mach/host_info.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/processor_info.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace system_monitor {
namespace {

std::uint64_t readUInt64Sysctl(const char* name) {
    std::uint64_t value = 0;
    std::size_t size = sizeof(value);
    if (::sysctlbyname(name, &value, &size, nullptr, 0) != 0) {
        throw std::system_error(errno, std::generic_category(),
            std::string("sysctlbyname failed for ") + name);
    }
    return value;
}

std::uint32_t readUInt32Sysctl(const char* name) {
    std::uint32_t value = 0;
    std::size_t size = sizeof(value);
    if (::sysctlbyname(name, &value, &size, nullptr, 0) != 0) {
        throw std::system_error(errno, std::generic_category(),
            std::string("sysctlbyname failed for ") + name);
    }
    return value;
}

std::uint64_t safeMultiply(std::uint64_t left, std::uint64_t right) {
    if (left == 0 || right == 0) {
        return 0;
    }
    if (left > std::numeric_limits<std::uint64_t>::max() / right) {
        return std::numeric_limits<std::uint64_t>::max();
    }
    return left * right;
}

std::string executeProcessListCommand() {
    constexpr const char* command =
        "LC_ALL=C ps -axo pid=,user=,%cpu=,%mem=,rss=,comm=,command=";
    std::array<char, 4096> buffer{};
    std::string output;
    FILE* pipe = ::popen(command, "r");
    if (pipe == nullptr) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "popen failed for ps");
    }

    while (::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        output.append(buffer.data());
    }

    const int status = ::pclose(pipe);
    if (status != 0) {
        throw std::runtime_error("ps command failed while reading processes");
    }

    return output;
}

} // namespace

SystemSnapshot MacOsSystemMetricsProvider::snapshot() {
    SystemSnapshot result;
    result.cpu = readCpu();
    result.memory = readMemory();
    result.disks = readDisks();
    result.system = readSystem();
    return result;
}

std::vector<ProcessInfo> MacOsSystemMetricsProvider::processes() {
    return parseProcessList(executeProcessListCommand());
}

CpuInfo MacOsSystemMetricsProvider::readCpu() {
    CpuInfo info;
    info.logicalCores = readUInt32Sysctl("hw.logicalcpu");

    std::array<double, 3> load{};
    if (::getloadavg(load.data(), static_cast<int>(load.size())) == 3) {
        info.load1 = load[0];
        info.load5 = load[1];
        info.load15 = load[2];
    }

    const auto current = readCpuTicks();
    std::lock_guard lock(cpuMutex_);

    if (previousCpuTicks_ &&
        current.active >= previousCpuTicks_->active &&
        current.idle >= previousCpuTicks_->idle) {
        const auto activeDelta = current.active - previousCpuTicks_->active;
        const auto idleDelta = current.idle - previousCpuTicks_->idle;
        const auto totalDelta = activeDelta + idleDelta;
        if (totalDelta != 0) {
            info.usagePercent =
                100.0 * static_cast<double>(activeDelta) /
                static_cast<double>(totalDelta);
        }
    } else {
        const auto total = current.active + current.idle;
        if (total != 0) {
            info.usagePercent =
                100.0 * static_cast<double>(current.active) /
                static_cast<double>(total);
        }
    }

    previousCpuTicks_ = current;
    info.usagePercent = std::clamp(info.usagePercent, 0.0, 100.0);

    return info;
}

MemoryInfo MacOsSystemMetricsProvider::readMemory() const {
    vm_statistics64_data_t vmStats{};
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    const auto status = ::host_statistics64(
        ::mach_host_self(),
        HOST_VM_INFO64,
        reinterpret_cast<host_info64_t>(&vmStats),
        &count);
    if (status != KERN_SUCCESS) {
        throw std::runtime_error("host_statistics64 failed");
    }

    vm_size_t pageSize = 0;
    if (::host_page_size(::mach_host_self(), &pageSize) != KERN_SUCCESS) {
        throw std::runtime_error("host_page_size failed");
    }

    MemoryInfo info;
    info.totalBytes = readUInt64Sysctl("hw.memsize");

    const auto availablePages =
        static_cast<std::uint64_t>(vmStats.free_count) +
        static_cast<std::uint64_t>(vmStats.inactive_count) +
        static_cast<std::uint64_t>(vmStats.speculative_count);
    info.availableBytes = std::min(
        info.totalBytes,
        safeMultiply(availablePages, static_cast<std::uint64_t>(pageSize)));
    info.usedBytes = info.totalBytes - info.availableBytes;

    return info;
}

std::vector<DiskInfo> MacOsSystemMetricsProvider::readDisks() const {
    struct statfs stats {};
    if (::statfs("/", &stats) != 0) {
        throw std::system_error(errno, std::generic_category(), "statfs failed for /");
    }

    DiskInfo disk;
    disk.device = stats.f_mntfromname;
    disk.mountPoint = stats.f_mntonname;
    disk.fileSystem = stats.f_fstypename;
    disk.totalBytes = safeMultiply(
        static_cast<std::uint64_t>(stats.f_blocks),
        static_cast<std::uint64_t>(stats.f_bsize));
    disk.availableBytes = safeMultiply(
        static_cast<std::uint64_t>(stats.f_bavail),
        static_cast<std::uint64_t>(stats.f_bsize));
    disk.availableBytes = std::min(disk.availableBytes, disk.totalBytes);
    disk.usedBytes = disk.totalBytes - disk.availableBytes;

    return {std::move(disk)};
}

SystemInfo MacOsSystemMetricsProvider::readSystem() const {
    SystemInfo info;

    std::array<char, 256> hostname{};
    if (::gethostname(hostname.data(), hostname.size()) != 0) {
        throw std::system_error(errno, std::generic_category(), "gethostname failed");
    }
    hostname.back() = '\0';
    info.hostname = hostname.data();

    struct utsname name {};
    if (::uname(&name) != 0) {
        throw std::system_error(errno, std::generic_category(), "uname failed");
    }

    info.operatingSystem = "macOS";
    info.kernel = std::string(name.sysname) + " " + name.release;
    info.architecture = name.machine;

    timeval bootTime{};
    std::size_t bootTimeSize = sizeof(bootTime);
    int mib[] = {CTL_KERN, KERN_BOOTTIME};
    if (::sysctl(mib, 2, &bootTime, &bootTimeSize, nullptr, 0) != 0) {
        throw std::system_error(errno, std::generic_category(), "sysctl(KERN_BOOTTIME) failed");
    }

    timeval now{};
    if (::gettimeofday(&now, nullptr) != 0) {
        throw std::system_error(errno, std::generic_category(), "gettimeofday failed");
    }
    if (now.tv_sec >= bootTime.tv_sec) {
        info.uptimeSeconds = static_cast<std::uint64_t>(now.tv_sec - bootTime.tv_sec);
    }

    return info;
}

MacOsSystemMetricsProvider::CpuTicks MacOsSystemMetricsProvider::readCpuTicks() const {
    natural_t processorCount = 0;
    processor_info_array_t processorInfo = nullptr;
    mach_msg_type_number_t processorInfoCount = 0;

    const auto status = ::host_processor_info(
        ::mach_host_self(),
        PROCESSOR_CPU_LOAD_INFO,
        &processorCount,
        &processorInfo,
        &processorInfoCount);
    if (status != KERN_SUCCESS) {
        throw std::runtime_error("host_processor_info failed");
    }

    CpuTicks ticks;
    const auto cpuLoad = reinterpret_cast<processor_cpu_load_info_t>(processorInfo);
    for (natural_t index = 0; index < processorCount; ++index) {
        ticks.active += static_cast<std::uint64_t>(cpuLoad[index].cpu_ticks[CPU_STATE_USER]);
        ticks.active += static_cast<std::uint64_t>(cpuLoad[index].cpu_ticks[CPU_STATE_SYSTEM]);
        ticks.active += static_cast<std::uint64_t>(cpuLoad[index].cpu_ticks[CPU_STATE_NICE]);
        ticks.idle += static_cast<std::uint64_t>(cpuLoad[index].cpu_ticks[CPU_STATE_IDLE]);
    }

    const auto byteCount = static_cast<vm_size_t>(processorInfoCount * sizeof(integer_t));
    ::vm_deallocate(
        ::mach_task_self(),
        reinterpret_cast<vm_address_t>(processorInfo),
        byteCount);

    return ticks;
}

} // namespace system_monitor